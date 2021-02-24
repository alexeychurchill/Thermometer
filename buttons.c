#include "buttons.h"
#include "gpio.h"
#include "utils.h"
#include "uart.h"

// 72MHz / 36000 = 2kHz ==> 1 Tick - 500us
#define HMI_BTN_TIM_PSC                                 35999u

#define HMI_BTN_PERIOD_PRESS_SINGLE                     70u
#define HMI_BTN_PERIOD_PRESS_POLL_MODE                  20u
#define HMI_BTN_PERIOD_PRESS_LONG                       1500u

#define HMI_BTN_TIM                                     TIM4
#define HMI_BTN_TIM_IRQ_N                               TIM4_IRQn

#define HMI_BTN_LEFT_GPIO                               GPIOB
#define HMI_BTN_LEFT_PIN                                5

#define HMI_BTN_ENTER_GPIO                              GPIOB
#define HMI_BTN_ENTER_PIN                               4

#define HMI_BTN_RIGHT_GPIO                              GPIOB
#define HMI_BTN_RIGHT_PIN                               3

static HmiBtnEvent_t s_event = {
        .btn = HMI_BTN_NONE,
        .type = HMI_BTN_EVENT_NONE
};

static HmiBtn_t s_btn = HMI_BTN_NONE;
static GPIO_TypeDef *s_btn_gpio = NULL;
static uint32_t s_btn_pin = 0;

static volatile uint32_t s_btn_pressed_dur = 0x0u;

// Internal functions prototypes

// EXTI control

static FORCE_INLINE void __enable_exti();
static FORCE_INLINE void __disable_exti();

// EXTI common

static FORCE_INLINE void __handle_exti(
        HmiBtn_t btn, GPIO_TypeDef *gpio, uint32_t pin, uint32_t pr_flag
);

// TIM control

static FORCE_INLINE void __tim_set_period(uint32_t period_ms);
static FORCE_INLINE void __tim_start();
static FORCE_INLINE void __tim_start_once();
static FORCE_INLINE void __tim_stop();

// TIM functions

static FORCE_INLINE void __tim_handle_tick();

// IRQ Handlers

void EXTI3_IRQHandler(void) {
    __handle_exti(HMI_BTN_RIGHT, HMI_BTN_RIGHT_GPIO, HMI_BTN_RIGHT_PIN, EXTI_PR_PIF3);
}

void EXTI4_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PIF4) {
        __handle_exti(HMI_BTN_ENTER, HMI_BTN_ENTER_GPIO, HMI_BTN_ENTER_PIN, EXTI_PR_PIF4);
    }
}

void EXTI9_5_IRQHandler(void) {
    if (EXTI->PR & EXTI_PR_PIF5) {
        __handle_exti(HMI_BTN_LEFT, HMI_BTN_LEFT_GPIO, HMI_BTN_LEFT_PIN, EXTI_PR_PIF5);
    }
}

void TIM4_IRQHandler(void) {
    if ((HMI_BTN_TIM->SR & TIM_SR_UIF) == TIM_SR_UIF) {
        __tim_handle_tick();
    }

    HMI_BTN_TIM->SR = 0x0;
}

// Functions

void hmi_btn_init() {
    // Buttons timer

    HMI_BTN_TIM->PSC = HMI_BTN_TIM_PSC;
    HMI_BTN_TIM->ARR = HMI_BTN_PERIOD_PRESS_SINGLE;
    HMI_BTN_TIM->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(HMI_BTN_TIM_IRQ_N);

    // Left button
    gpio_setup(HMI_BTN_LEFT_GPIO, HMI_BTN_LEFT_PIN, GPIO_IN_FLOATING, GPIO_MODE_IN);
    AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI5_PB;
    EXTI->FTSR |= EXTI_FTSR_FT5;
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    // Enter button
    gpio_setup(HMI_BTN_ENTER_GPIO, HMI_BTN_ENTER_PIN, GPIO_IN_FLOATING, GPIO_MODE_IN);
    AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI4_PB;
    EXTI->FTSR |= EXTI_FTSR_FT4;
    NVIC_EnableIRQ(EXTI4_IRQn);

    // Right button
    gpio_setup(HMI_BTN_RIGHT_GPIO, HMI_BTN_RIGHT_PIN, GPIO_IN_FLOATING, GPIO_MODE_IN);
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI3_PB;
    EXTI->FTSR |= EXTI_FTSR_FT3;
    NVIC_EnableIRQ(EXTI3_IRQn);

    __enable_exti();
}

const bool hmi_btn_has_event() {
    return s_event.type != HMI_BTN_EVENT_NONE;
}

const HmiBtnEvent_t hmi_btn_poll_event() {
    HmiBtnEvent_t evt = s_event;
    s_event = (HmiBtnEvent_t) {
        .btn = HMI_BTN_NONE,
        .type = HMI_BTN_EVENT_NONE
    };
    return evt;
}

// Internal functions implementations
/**
 * TODO: Improve, make more safe
 */
static FORCE_INLINE void __enable_exti() {
    EXTI->PR |= EXTI_PR_PIF3;
    EXTI->IMR |= EXTI_IMR_IM3;

    EXTI->PR |= EXTI_PR_PIF4;
    EXTI->IMR |= EXTI_IMR_IM4;

    EXTI->PR |= EXTI_PR_PIF5;
    EXTI->IMR |= EXTI_IMR_IM5;
}

/**
 * TODO: Improve, make more safe
 */
static FORCE_INLINE void __disable_exti() {
    EXTI->IMR &= ~EXTI_IMR_IM3;
    EXTI->IMR &= ~EXTI_IMR_IM4;
    EXTI->IMR &= ~EXTI_IMR_IM5;
}

static FORCE_INLINE void __handle_exti(
        HmiBtn_t btn, GPIO_TypeDef *gpio, uint32_t pin, uint32_t pr_flag
) {

    __disable_exti();

    EXTI->PR |= pr_flag;

    if (gpio_read(gpio, pin)) {
        // Protection from accidental Falling Front when button is being released
        // TODO: Consider replacing with timer timeout
        // after button is being released
        __enable_exti();
        return;
    }

    __tim_stop();
    __tim_set_period(HMI_BTN_PERIOD_PRESS_SINGLE);

    s_btn_pressed_dur = 0x0u;

    s_btn = btn;
    s_btn_gpio = gpio;
    s_btn_pin = pin;

    __tim_start_once();
}

static FORCE_INLINE void __tim_set_period(uint32_t period_ms) {
    if ((HMI_BTN_TIM->CR1 & TIM_CR1_CEN) != TIM_CR1_CEN) {
        HMI_BTN_TIM->ARR = (2u * period_ms) - 1u; // 1 Tick is 500us
    }
}

static FORCE_INLINE void __tim_start() {
    HMI_BTN_TIM->CR1 &= ~TIM_CR1_OPM;
    HMI_BTN_TIM->CR1 |= TIM_CR1_CEN;
}

static FORCE_INLINE void __tim_start_once() {
    HMI_BTN_TIM->CR1 |= TIM_CR1_OPM;
    HMI_BTN_TIM->CR1 |= TIM_CR1_CEN;
}

static FORCE_INLINE void __tim_stop() {
    HMI_BTN_TIM->CR1 &= ~TIM_CR1_CEN;
}

static FORCE_INLINE void __tim_handle_tick() {
    bool btn_pressed = !gpio_read(s_btn_gpio, s_btn_pin);
    bool opm = (HMI_BTN_TIM->CR1 & TIM_CR1_OPM) == TIM_CR1_OPM;

    if (btn_pressed && opm) {
        s_btn_pressed_dur = HMI_BTN_PERIOD_PRESS_SINGLE;
        // Switch to the poll mode
        __tim_set_period(HMI_BTN_PERIOD_PRESS_POLL_MODE);
        __tim_start();
    } else if (btn_pressed && !opm && (s_btn_pressed_dur < HMI_BTN_PERIOD_PRESS_LONG)) {
        s_btn_pressed_dur += HMI_BTN_PERIOD_PRESS_POLL_MODE;
    } else if (btn_pressed && !opm) {
        __tim_stop();

        s_event.type = HMI_BTN_EVENT_LONG_PRESS;
        s_event.btn = s_btn;
        s_btn_gpio = NULL;

        __enable_exti();
    } else {
        __tim_stop();

        // Probably, should be removed after introducing of timeout
        // on EXTI when button is being released
        if (s_btn_pressed_dur > 0u) {
            s_event.type = HMI_BTN_EVENT_PRESS;
            s_event.btn = s_btn;
            s_btn_gpio = NULL;
        }

        __enable_exti();
    }
}
