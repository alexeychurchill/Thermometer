#include "onewire.h"
#include <stdint.h>
#include "stm32f1xx.h"
#include "gpio.h"


#define ONE_WIRE_TIM_PSC            71      // 72


#define ONE_WIRE_RST_TIMEOUT        2999    // 3000 us
#define ONE_WIRE_RST_PULSE          499     // 500 us
#define ONE_WIRE_RST_PAUSE_MIN      14      // 15 us
#define ONE_WIRE_RST_PAUSE_MAX      59      // 60 us
#define ONE_WIRE_RST_PRESENCE_MIN   59      // 60 us
#define ONE_WIRE_RST_PRESENCE_MAX   239     // 240 us
#define OW_RST_TOTAL_PRSC_MIN       74      // 75 us
#define OW_RST_TOTAL_PRSC_MAX       299     // 300 us


#define TIM_CCMR1_OW_OUT_CONFIG (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1)
#define TIM_CCER_OW_OUT_ENABLE (TIM_CCER_CC1E | TIM_CCER_CC1P)

#define TIM_CCMR1_OW_IN_CONFIG (TIM_CCMR1_CC2S_1)
#define TIM_CCER_OW_IN_ENABLE (TIM_CCER_CC2E)
#define TIM_DIER_OW_IN_ISR (TIM_DIER_CC2IE | TIM_DIER_UIE)

#define TIM_DIER_OW_RST_RESET_TX (TIM_DIER_CC1IE)
#define TIM_DIER_OW_RST_PRESENCE_RX (TIM_DIER_CC2IE | TIM_DIER_UIE)


static one_wire_status_t bus_status = OW_STS_IDLE;

static one_wire_error_t bus_error = OW_ERR_NONE;

static volatile uint32_t ow_rst_presence_timeout = 0;
static volatile uint32_t ow_rst_presence_start = 0;
static volatile uint32_t ow_rst_presence_stop = 0;

void FORCE_INLINE ow_bus_set_idle() {
    gpio_set(GPIOA, 0);
    gpio_setup(GPIOA, 0, GPIO_OUT_OD, GPIO_MODE_OUT_50MHZ);
}

void FORCE_INLINE ow_bus_set_active() {
    gpio_setup(GPIOA, 0, GPIO_OUT_AF_OD, GPIO_MODE_OUT_50MHZ);
}

void FORCE_INLINE ow_bus_disable_tx() {
    NVIC_DisableIRQ(TIM2_IRQn);
    ow_bus_set_idle();
    TIM2 -> CR1 = 0;
}

static FORCE_INLINE uint32_t ow_bus_rst_prsc_len_ok(uint32_t start, uint32_t stop) {
    uint32_t len = stop - start; 
    if (len >= OW_RST_TOTAL_PRSC_MIN && len <= OW_RST_TOTAL_PRSC_MAX) {
        return 1; 
    }

    return 0;
}

static FORCE_INLINE void ow_bus_rst_handle_state() {
    if (bus_status != OW_STS_RESET_IN_PROGRESS) {
        return; 
    }

    if (ow_rst_presence_timeout) {
        ow_bus_disable_tx();
        bus_error = OW_ERR_PRESENCE_TIMEOUT; 
        bus_status = OW_STS_RESET_DONE; 
        return;
    }

    if (ow_rst_presence_start > 0 && ow_rst_presence_stop > 0) {
        ow_bus_disable_tx();
        uint32_t presence_len = ow_rst_presence_stop - ow_rst_presence_start; 
        if (presence_len >= OW_RST_TOTAL_PRSC_MIN && presence_len <= OW_RST_TOTAL_PRSC_MAX) {
            bus_error = OW_ERR_NONE;
            bus_status = OW_STS_RESET_DONE;
        } else {
            bus_error = OW_ERR_PRESENCE_WRONG_TIMING;
            bus_status = OW_STS_RESET_DONE;
        }
    }
}

void TIM2_IRQHandler(void) {
    if (TIM2 -> SR & TIM_SR_CC2IF) {
        ow_rst_presence_start = ow_rst_presence_stop;
        ow_rst_presence_stop = TIM2 -> CCR2;
        TIM2 -> SR = 0;
    }

    if (TIM2 -> SR & TIM_SR_UIF) {
        ow_rst_presence_timeout = 1;
        TIM2 -> SR = 0;
    }

    ow_bus_rst_handle_state();
}

one_wire_status_t ow_status_get() {
    return bus_status;
}

one_wire_error_t ow_error_get() {
    return bus_error;
}

void ow_error_reset() {
    bus_error = OW_ERR_NONE;
}

void one_wire_init() {
    RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;
    ow_bus_set_idle();
}

void one_wire_reset() {
    ow_rst_presence_timeout = 0;
    ow_rst_presence_start = 0;
    ow_rst_presence_stop = 0;

    TIM2 -> CR1 = 0;

    TIM2 -> DIER = 0;
    TIM2 -> CNT = 0x0;
    TIM2 -> PSC = ONE_WIRE_TIM_PSC;
    TIM2 -> ARR = ONE_WIRE_RST_TIMEOUT;
    TIM2 -> EGR |= TIM_EGR_UG; 

    TIM2 -> CCR1 = ONE_WIRE_RST_PULSE;
    TIM2 -> CCMR1 |= TIM_CCMR1_OW_OUT_CONFIG;
    TIM2 -> CCER |= TIM_CCER_OW_OUT_ENABLE;

    TIM2 -> CCMR1 |= TIM_CCMR1_OW_IN_CONFIG;
    TIM2 -> CCER |= TIM_CCER_OW_IN_ENABLE;

    ow_bus_set_active();
    bus_error = OW_ERR_NONE;
    bus_status = OW_STS_RESET_IN_PROGRESS;
    TIM2 -> CR1 = (TIM_CR1_CEN | TIM_CR1_OPM);
    TIM2 -> SR = 0;
    TIM2 -> DIER = TIM_DIER_OW_IN_ISR;

    NVIC_EnableIRQ(TIM2_IRQn);
}
