#include "onewire.h"
#include <stdint.h>
#include "stm32f1xx.h"
#include "gpio.h"


#define ONE_WIRE_TIM_PSC            71      // 72
#define OW_TIM_PSC                  71      // 72


#define ONE_WIRE_RST_TIMEOUT        2999    // 3000 us
#define ONE_WIRE_RST_PULSE          499     // 500 us
#define ONE_WIRE_RST_PAUSE_MIN      14      // 15 us
#define ONE_WIRE_RST_PAUSE_MAX      59      // 60 us
#define ONE_WIRE_RST_PRESENCE_MIN   59      // 60 us
#define ONE_WIRE_RST_PRESENCE_MAX   239     // 240 us
#define OW_RST_TOTAL_PRSC_MIN       74      // 75 us
#define OW_RST_TOTAL_PRSC_MAX       299     // 300 us


#define OW_BUFFER_TX_LEN_BITS       9       // 9 bits: 8 bit data + 1 dummy bit
#define OW_BUFFER_RX_LEN_BITS       8       // 8 bits: 8 bit data

#define TIM_CCMR1_OW_OUT_CONFIG     (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1)
#define TIM_CCER_OW_OUT_ENABLE      (TIM_CCER_CC1E | TIM_CCER_CC1P)

#define TIM_CCMR1_OW_IN_CONFIG      (TIM_CCMR1_CC2S_1)
#define TIM_CCER_OW_IN_ENABLE       (TIM_CCER_CC2E)
#define TIM_DIER_OW_IN_ISR          (TIM_DIER_CC2IE | TIM_DIER_UIE)

#define TIM_DIER_OW_RST_RESET_TX    (TIM_DIER_CC1IE)
#define TIM_DIER_OW_RST_PRESENCE_RX (TIM_DIER_CC2IE | TIM_DIER_UIE)


// PWM Mode 1 (RM0008, p. 414)
#define OW_TIM_CCMR1_PWM            (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1)
// Timer Channel 1 preload enabled
#define OW_TX_TIM_CCMR1             (OW_TIM_CCMR1_PWM | TIM_CCMR1_OC1PE)
// Timer Channel 1 enabled and has inverted polarity 
#define OW_TX_TIM_CCER              (TIM_CCER_CC1E | TIM_CCER_CC1P)
// ARR preload enable
#define OW_TX_TIM_CR1               (TIM_CR1_ARPE)
#define OW_TX_TIM_CR2               (TIM_CR2_CCDS)
#define OW_TX_TIM_DIER              (TIM_DIER_UDE)

#define OW_TX_SLOT_LEN              ((uint16_t) 70)      // 70 us
#define OW_TX_DUMMY                 ((uint16_t) 0)       // 0 us
#define OW_TX_LOW_LEN               ((uint16_t) 65)      // 65 us
#define OW_TX_HIGH_LEN              ((uint16_t) 13)      // 13 us

#define OW_TX_DMA_CCR               (DMA_CCR_MINC | \
    DMA_CCR_DIR | \
    DMA_CCR_PSIZE_0 | \
    DMA_CCR_MSIZE_0 | \
    DMA_CCR_TCIE)

#define OW_RX_DMA_CCR               (DMA_CCR_MINC | \
    DMA_CCR_PSIZE_0 | \
    DMA_CCR_MSIZE_0 | \
    DMA_CCR_TCIE)


static one_wire_status_t bus_status = OW_STS_IDLE;

static one_wire_error_t bus_error = OW_ERR_NONE;

static volatile uint32_t ow_rst_presence_timeout = 0;
static volatile uint32_t ow_rst_presence_start = 0;
static volatile uint32_t ow_rst_presence_stop = 0;

static volatile uint16_t ow_buffer_tx[OW_BUFFER_TX_LEN_BITS];
static volatile uint16_t ow_buffer_rx[OW_BUFFER_RX_LEN_BITS];



void FORCE_INLINE ow_bus_set_idle() {
    gpio_set(GPIOA, 0);
    gpio_setup(GPIOA, 0, GPIO_OUT_OD, GPIO_MODE_OUT_50MHZ);
}

void FORCE_INLINE ow_bus_set_active() {
    gpio_setup(GPIOA, 0, GPIO_OUT_AF_OD, GPIO_MODE_OUT_50MHZ);
}


// TODO: Replace with ow_tim_stop
void FORCE_INLINE ow_bus_disable_tx() {
    NVIC_DisableIRQ(TIM2_IRQn);
    ow_bus_set_idle();
    TIM2 -> CR1 = 0;
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

static FORCE_INLINE void ow_put_to_tx_buf(uint8_t byte) {
    for (uint32_t i = 0; i < 8; i++) {
        uint8_t bit_mask = 0x1 << (7 - i);
        uint8_t bit_value = byte & bit_mask;
        uint16_t duration = bit_value ? OW_TX_HIGH_LEN : OW_TX_LOW_LEN;
        ow_buffer_tx[7 - i] = duration;
    }
    ow_buffer_tx[OW_BUFFER_TX_LEN_BITS - 1] = OW_TX_DUMMY; 
}

void TIM2_IRQHandler(void) {
    if (bus_status == OW_STS_RESET_IN_PROGRESS) {
        if (TIM2 -> SR & TIM_SR_CC2IF) {
            ow_rst_presence_start = ow_rst_presence_stop;
            ow_rst_presence_stop = TIM2 -> CCR2;
            TIM2 -> SR = 0;
        }

        if (TIM2 -> SR & TIM_SR_UIF) {
            TIM2 -> CR1 = 0;
            ow_rst_presence_timeout = 1;
            TIM2 -> SR = 0;
        }

        ow_bus_rst_handle_state();
    }

    if (bus_status == OW_STS_SEND_IN_PROGRESS) {
        gpio_set(GPIOA, 0);
        gpio_setup(GPIOA, 0, GPIO_OUT_OD, GPIO_MODE_OUT_50MHZ);
        TIM2 -> CR1 = 0;
        TIM2 -> SR = 0;
        bus_status = OW_STS_SEND_DONE; 
        bus_error = OW_ERR_NONE; 
    }
}

void DMA1_Channel2_IRQHandler(void) {
    TIM2 -> SR = 0;
    TIM2 -> DIER = TIM_DIER_UIE;
    NVIC_EnableIRQ(TIM2_IRQn);

    NVIC_DisableIRQ(DMA1_Channel2_IRQn);

    DMA1 -> IFCR = DMA_IFCR_CTCIF2;
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
    bus_status = OW_STS_IDLE;
    RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;
    ow_bus_set_idle();
}

#define TIM_ARR_PRELOAD             (TIM_CR1_ARPE)
#define TIM_CH1_PRELOAD             (TIM_CCMR1_OC1PE)

#define TIM_CH1_OUT_PWM1            (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1)
#define TIM_CH2_IN_TI1              (TIM_CCMR1_CC2S_1)
#define TIM_CH1_EN_INV              (TIM_CCER_CC1E | TIM_CCER_CC1P)
#define TIM_CH2_EN                  (TIM_CCER_CC2E)

#define TIM_START(tim)              (tim -> CR1 |= TIM_CR1_CEN)
#define TIM_START_ONCE(tim)         (tim -> CR1 |= (TIM_CR1_CEN | TIM_CR1_OPM))


#define OW_TIM_CH_RESET_CONFIG      (TIM_CH1_OUT_PWM1 | TIM_CH2_IN_TI1)
#define OW_TIM_CH_RESET_EN          (TIM_CH1_EN_INV | TIM_CH2_EN)



void ow_tim_stop(TIM_TypeDef *tim);


void one_wire_reset() {
    ow_rst_presence_timeout = 0;
    ow_rst_presence_start = 0;
    ow_rst_presence_stop = 0;

    ow_tim_stop(TIM2);

    TIM2 -> PSC = ONE_WIRE_TIM_PSC;
    TIM2 -> ARR = ONE_WIRE_RST_TIMEOUT;
    TIM2 -> EGR |= TIM_EGR_UG; 

    TIM2 -> CCR1 = ONE_WIRE_RST_PULSE;

    TIM2 -> CCMR1 |= OW_TIM_CH_RESET_CONFIG;
    TIM2 -> CCER |= OW_TIM_CH_RESET_EN;

    bus_error = OW_ERR_NONE;
    bus_status = OW_STS_RESET_IN_PROGRESS;

    ow_bus_set_active();
    TIM_START_ONCE(TIM2);
    TIM2 -> SR = 0x0;
    TIM2 -> DIER = TIM_DIER_OW_IN_ISR;

    NVIC_EnableIRQ(TIM2_IRQn);
}

void ow_tim_init_dma(TIM_TypeDef *tim) {
    tim -> CR1 = TIM_CR1_ARPE;
    tim -> DIER = (TIM_DIER_UDE | TIM_DIER_CC2DE);
    tim -> CCMR1 = (TIM_CH1_OUT_PWM1 | TIM_CH2_IN_TI1 | TIM_CCMR1_OC1PE);
    tim -> CCER = (TIM_CH1_EN_INV | TIM_CH2_EN);
}

void ow_tim_init_timing(TIM_TypeDef *tim, uint16_t slot_len, uint16_t init_pulse_len) {
    tim -> PSC = OW_TIM_PSC;
    tim -> ARR = slot_len;
    tim -> ARR = slot_len;
    tim -> CCR1 = init_pulse_len;
}

void ow_dma_init_tx(DMA_Channel_TypeDef *dma, TIM_TypeDef *tim, uint16_t *buf, uint16_t len) {
    dma -> CCR = OW_TX_DMA_CCR;
    dma -> CPAR = (uint32_t)(&(tim -> CCR1));
    dma -> CMAR = (uint32_t)(buf);
    dma -> CNDTR = len;
}

void ow_dma_init_rx(DMA_Channel_TypeDef *dma, TIM_TypeDef *tim, uint16_t *buf, uint16_t len) {
    dma -> CCR = OW_RX_DMA_CCR;
    dma -> CPAR = (uint32_t)(&(tim -> CCR2));
    dma -> CMAR = (uint32_t)(buf);
    dma -> CNDTR = len;
}

#define OW_DMA_STOP(dma_n, channel_n) {\
        NVIC_DisableIRQ(DMA## dma_n## _Channel## channel_n## _IRQn);\
        DMA## dma_n## _Channel## channel_n -> CCR &= (~DMA_CCR_EN);\
    }

#define OW_DMA_START(dma_n, channel_n) {\
        NVIC_EnableIRQ(DMA## dma_n## _Channel## channel_n## _IRQn);\
        DMA## dma_n## _Channel## channel_n -> CCR |= DMA_CCR_EN;\
    }

void ow_tim_stop(TIM_TypeDef *tim) {
    ow_bus_set_idle();
    NVIC_DisableIRQ(TIM2_IRQn);
    tim -> CR1 = 0x0; 
    tim -> CNT = 0x0;
    tim -> SR = 0x0;
}

void ow_tim_start(TIM_TypeDef *tim) {
    ow_bus_set_active();
    tim -> CR1 |= TIM_CR1_CEN;
}

void ow_tx_byte(uint8_t data) {
    ow_tim_stop(TIM2);
    OW_DMA_STOP(1, 2);

    ow_put_to_tx_buf(data);

    ow_tim_init_dma(TIM2);
    ow_tim_init_timing(TIM2, OW_TX_SLOT_LEN, ow_buffer_tx[0]);

    ow_dma_init_tx(DMA1_Channel2, TIM2, (&ow_buffer_tx[1]), OW_BUFFER_TX_LEN_BITS - 1);

    bus_error = OW_ERR_NONE;
    bus_status = OW_STS_SEND_IN_PROGRESS;

    OW_DMA_START(1, 2);
    ow_tim_start(TIM2);
}
