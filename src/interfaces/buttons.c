#include "buttons.h"
#include "gpio.h"
#include "util/utils.h"

// 72MHz / 36000 = 2kHz ==> 1 Tick - 500us
#define HMI_BTN_TIM_PSC                                 35999u

#define HMI_BTN_TIM                                     TIM4
#define HMI_BTN_TIM_IRQ_N                               TIM4_IRQn

#define HMI_BTN_TIM_PERIOD                              1u    //(2u - 1) ticks, 1 Tick = 500us
#define HMI_BTN_DEBOUNCE                                70u   //ms
#define HMI_BTN_PERIOD_PRESS_SHORT                      70u   //ms
#define HMI_BTN_PERIOD_PRESS_LONG                       1500u //ms

static GPIO_TypeDef* s_button_gpio[] = {
        [HMI_BTN_LEFT]  = GPIOB,
        [HMI_BTN_ENTER] = GPIOB,
        [HMI_BTN_RIGHT] = GPIOB,
};

static uint32_t s_button_pin[] = {
        [HMI_BTN_LEFT]  = 5u,
        [HMI_BTN_ENTER] = 4u,
        [HMI_BTN_RIGHT] = 3u,
};

static uint32_t s_debounce_period = 0u;
static uint32_t s_press_duration = 0u;
static HmiBtn_t s_button = HMI_BTN_NONE;
static HmiBtnEventType_t s_button_event = HMI_BTN_EVENT_NONE;

// Internal functions prototypes

static void enable_exti() {
    EXTI->PR |= EXTI_PR_PIF3;
    EXTI->IMR |= EXTI_IMR_IM3;

    EXTI->PR |= EXTI_PR_PIF4;
    EXTI->IMR |= EXTI_IMR_IM4;

    EXTI->PR |= EXTI_PR_PIF5;
    EXTI->IMR |= EXTI_IMR_IM5;
}

// Functions

void hmi_btn_init() {
    // Buttons timer

    HMI_BTN_TIM->PSC = HMI_BTN_TIM_PSC;
    HMI_BTN_TIM->ARR = HMI_BTN_TIM_PERIOD;
    HMI_BTN_TIM->DIER |= TIM_DIER_UIE;
    NVIC_EnableIRQ(HMI_BTN_TIM_IRQ_N);
    HMI_BTN_TIM->CR1 |= TIM_CR1_CEN;

    // Left button
    gpio_setup(s_button_gpio[HMI_BTN_LEFT], s_button_pin[HMI_BTN_LEFT], GPIO_IN_FLOATING, GPIO_MODE_IN);
    AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI5_PB;
    EXTI->FTSR |= EXTI_FTSR_FT5;
    NVIC_EnableIRQ(EXTI9_5_IRQn);

    // Enter button
    gpio_setup(s_button_gpio[HMI_BTN_ENTER], s_button_pin[HMI_BTN_ENTER], GPIO_IN_FLOATING, GPIO_MODE_IN);
    AFIO->EXTICR[1] |= AFIO_EXTICR2_EXTI4_PB;
    EXTI->FTSR |= EXTI_FTSR_FT4;
    NVIC_EnableIRQ(EXTI4_IRQn);

    // Right button
    gpio_setup(s_button_gpio[HMI_BTN_RIGHT], s_button_pin[HMI_BTN_RIGHT], GPIO_IN_FLOATING, GPIO_MODE_IN);
    AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
    AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI3_PB;
    EXTI->FTSR |= EXTI_FTSR_FT3;
    NVIC_EnableIRQ(EXTI3_IRQn);

    enable_exti();
}

void hmi_btn_on_press(const HmiBtn_t button) {
    if (hmi_btn_has_event() || s_debounce_period > 0u) {
        return;
    }

    s_button_event = HMI_BTN_EVENT_NONE;
    s_button = button;
    s_debounce_period = HMI_BTN_DEBOUNCE;
    s_press_duration = 0u;
}

void hmi_btn_on_tick() {
    if (s_debounce_period > 0u) {
        s_debounce_period--;
    }

    if (hmi_btn_has_event()) {
        return;
    }

    bool is_released = gpio_read(s_button_gpio[s_button], s_button_pin[s_button]);
    bool is_more_short_press = s_press_duration >= HMI_BTN_PERIOD_PRESS_SHORT;
    bool is_more_long_press = s_press_duration >= HMI_BTN_PERIOD_PRESS_LONG;

    if (is_released && !is_more_short_press) {
        s_button = HMI_BTN_NONE;
        s_button_event = HMI_BTN_EVENT_NONE;
        return;
    } else if (is_released && !is_more_long_press) {
        s_button_event = HMI_BTN_EVENT_PRESS;
        return;
    } else if (is_more_long_press) {
        s_button_event = HMI_BTN_EVENT_LONG_PRESS;
        return;
    }

    s_press_duration++;
}

bool hmi_btn_has_event() {
    return (s_button != HMI_BTN_NONE) && (s_button_event != HMI_BTN_EVENT_NONE);
}

HmiBtnEvent_t hmi_btn_poll_event() {
    bool is_event_present = hmi_btn_has_event();
    HmiBtnEvent_t event = {
            .btn = is_event_present ? s_button : HMI_BTN_NONE,
            .type = is_event_present ? s_button_event : HMI_BTN_EVENT_NONE,
    };

    if (is_event_present) {
        s_button = HMI_BTN_NONE;
        s_button_event = HMI_BTN_EVENT_NONE;
    }

    return event;
}
