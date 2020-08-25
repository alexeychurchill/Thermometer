#include "onewire.h"
#include <stdint.h>
#include "stm32f1xx.h"
#include "gpio.h"


#define OW_TIM_PSC                  71      // 72

#define OW_RESET_SLOT_LEN           2999    // 3000 us
#define OW_RESET_PULSE_LEN          499     // 500 us
#define OW_RESET_ACK_LEN_MIN        74      // 75 us
#define OW_RESET_ACK_LEN_MAX        299     // 300 us

#define OW_BUFFER_TX_LEN_BITS       89      // 11 bytes: 11 * 8 bit data + 1 dummy bit
#define OW_BUFFER_RX_LEN_BITS       88      // 11 bytes: 11 * 8 bit data

#define TIM_CCMR1_OW_OUT_CONFIG     (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1)
#define TIM_CCER_OW_OUT_ENABLE      (TIM_CCER_CC1E | TIM_CCER_CC1P)

#define TIM_CCMR1_OW_IN_CONFIG      (TIM_CCMR1_CC2S_1)
#define TIM_CCER_OW_IN_ENABLE       (TIM_CCER_CC2E)
#define TIM_DIER_OW_IN_ISR          (TIM_DIER_CC2IE | TIM_DIER_UIE)

#define TIM_DIER_OW_RST_RESET_TX    (TIM_DIER_CC1IE)
#define TIM_DIER_OW_RST_PRESENCE_RX (TIM_DIER_CC2IE | TIM_DIER_UIE)

#define OW_SLOT_LEN                 ((uint16_t) 70)      // 70 us
#define OW_TX_DUMMY                 ((uint16_t) 0)       // 0 us
#define OW_TX_LOW_LEN               ((uint16_t) 65)      // 65 us
#define OW_TX_HIGH_LEN              ((uint16_t) 13)      // 13 us
#define OW_RX_MARKER_LEN            ((uint16_t) 13)      // 13 us





#define TIM_CH1_OUT_PWM1            (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1)
#define TIM_CH2_IN_TI1              (TIM_CCMR1_CC2S_1)
#define TIM_CH1_EN_INV              (TIM_CCER_CC1E | TIM_CCER_CC1P)
#define TIM_CH2_EN                  (TIM_CCER_CC2E)

#define TIM_START(tim)              (tim -> CR1 |= TIM_CR1_CEN)
#define TIM_START_ONCE(tim)         (tim -> CR1 |= (TIM_CR1_CEN | TIM_CR1_OPM))


#define OW_TIM_CH_RESET_CONFIG      (TIM_CH1_OUT_PWM1 | TIM_CH2_IN_TI1)
#define OW_TIM_CH_RESET_EN          (TIM_CH1_EN_INV | TIM_CH2_EN)





#define OW_DMA_CCR_BASE             (DMA_CCR_MINC | DMA_CCR_PSIZE_0 | DMA_CCR_MSIZE_0)
#define OW_DMA_CCR_TX               (OW_DMA_CCR_BASE | DMA_CCR_DIR | DMA_CCR_TCIE)
#define OW_DMA_CCR_RX               (OW_DMA_CCR_BASE)



static one_wire_status_t bus_status = OW_STS_IDLE;

static one_wire_error_t bus_error = OW_ERR_NONE;

static volatile uint32_t ow_rst_presence_timeout = 0;
static volatile uint32_t ow_rst_presence_start = 0;
static volatile uint32_t ow_rst_presence_stop = 0;

static volatile uint16_t ow_buffer_tx[OW_BUFFER_TX_LEN_BITS];
static volatile uint16_t ow_buffer_rx[OW_BUFFER_RX_LEN_BITS];



void FORCE_INLINE ow_bus_set_idle() {
    // gpio_set(GPIOA, 0);
    // gpio_setup(GPIOA, 0, GPIO_OUT_OD, GPIO_MODE_OUT_50MHZ);
}

void FORCE_INLINE ow_bus_set_active() {
    // gpio_setup(GPIOA, 0, GPIO_OUT_AF_OD, GPIO_MODE_OUT_50MHZ);
}

typedef struct OwBusState {
    OW_BS_IDLE, 
    OW_BS_RESET_PULSE_TX, 
    OW_BS_RESET_PRESENCE_RX, 
    OW_BS_DATA_TX, 
    OW_BS_DATA_RX
} OwBusState_t;

typedef struct OwBusEvent {
    OW_BE_START, 
    OW_BE_IN_RISE, 
    OW_BE_TIM_UPD, 
    OW_BE_TX_DONE, 
    OW_BE_RX_DONE
} OwBusEvent_t;

typedef struct OwBusResult { 
    OW_BR_NONE, 
    OW_BR_RESET_OK,
    OW_BR_TX_OK, 
    OW_BR_RX_OK
} OwBusResult_t;


void ow_tim_free_bus(TIM_TypeDef *tim) {
    tim -> ARR = 0x0;
    tim -> CCR1 = 0x0;
    tim -> CNT = 0x0;
}

void ow_tim_force_update(TIM_TypeDef *tim) {
    tim -> EGR |= TIM_EGR_UG;
}

static FORCE_INLINE void ow_put_to_tx_buf(uint8_t byte) {
    for (uint32_t i = 0; i < 8; i++) {
        uint8_t bit_mask = 0x1 << (7 - i);
        uint8_t bit_value = byte & bit_mask;
        uint16_t duration = bit_value ? OW_TX_HIGH_LEN : OW_TX_LOW_LEN;
        ow_buffer_tx[i] = duration;
    }
    ow_buffer_tx[8] = OW_TX_DUMMY; 
}

void TIM2_IRQHandler(void) {
    if (TIM2 -> SR & TIM_SR_CC2IF) {
        ow_rst_presence_start = ow_rst_presence_stop;
        ow_rst_presence_stop = TIM2 -> CCR2;
        TIM2 -> SR = 0x0;

        // TODO: Chech actual lenght!
        if (ow_rst_presence_start != 0 && ow_rst_presence_stop != 0 && ow_rst_presence_stop > ow_rst_presence_start) {
            NVIC_DisableIRQ(TIM2_IRQn);
            TIM2 -> DIER = 0x0;
            TIM2 -> EGR = TIM_EGR_UG;
            TIM2 -> SR = 0x0;
        }
    }

    if (TIM2 -> SR & TIM_SR_UIF) {
        NVIC_DisableIRQ(TIM2_IRQn);
        TIM2 -> DIER = 0x0;
        TIM2 -> SR = 0x0;
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

void ow_tim_init(TIM_TypeDef *tim) {
    tim -> CR1 = TIM_CR1_ARPE;
    tim -> CCMR1 = (TIM_CH1_OUT_PWM1 | TIM_CH2_IN_TI1 | TIM_CCMR1_OC1PE);
    tim -> CCER = (TIM_CH1_EN_INV | TIM_CH2_EN);
    tim -> PSC = OW_TIM_PSC;
    tim -> ARR = OW_SLOT_LEN;
    tim -> CCR1 = 0x0;
    tim -> SR = 0x0;
}

void one_wire_init() {
    bus_status = OW_STS_IDLE;
    RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;
    ow_bus_set_idle();

    ow_tim_init(TIM2);
    TIM2 -> CR1 |= TIM_CR1_CEN;

    gpio_setup(GPIOA, 0, GPIO_OUT_AF_OD, GPIO_MODE_OUT_50MHZ);
}

void ow_tim_stop(TIM_TypeDef *tim);


void one_wire_reset() {
    ow_rst_presence_timeout = 0;
    ow_rst_presence_start = 0;
    ow_rst_presence_stop = 0;

    TIM2 -> ARR = OW_RESET_SLOT_LEN;
    TIM2 -> CCR1 = OW_RESET_PULSE_LEN;
    TIM2 -> EGR = TIM_EGR_UG;
    TIM2 -> SR = 0x0;
    TIM2 -> ARR = OW_SLOT_LEN; 
    TIM2 -> CCR1 = OW_TX_DUMMY;
    TIM2 -> DIER = TIM_DIER_UIE | TIM_DIER_CC2IE;
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
    dma -> CCR = OW_DMA_CCR_TX;
    dma -> CPAR = (uint32_t)(&(tim -> CCR1));
    dma -> CMAR = (uint32_t)(buf);
    dma -> CNDTR = len;
}

void ow_dma_init_rx(DMA_Channel_TypeDef *dma, TIM_TypeDef *tim, uint16_t *buf, uint16_t len) {
    dma -> CCR = OW_DMA_CCR_RX;
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

// void ow_setup_transceiver(uint16_t *tx_buffer, uint16_t *rx_buffer, uint16_t len) {
//     ow_tim_init_dma(TIM2);
//     ow_tim_init_timing(TIM2, OW_SLOT_LEN, tx_buffer[0]);
//     ow_dma_init_tx(DMA1_Channel2, TIM2, (&tx_buffer[1]), len - 1);
//     ow_dma_init_rx(DMA1_Channel7, TIM2, (&rx_buffer[0]), len);
// }

void ow_tx_byte(uint8_t data) {
    ow_tim_stop(TIM2);
    OW_DMA_STOP(1, 2);

    ow_put_to_tx_buf(data);

    ow_tim_init_dma(TIM2);
    ow_tim_init_timing(TIM2, OW_SLOT_LEN, ow_buffer_tx[0]);
    ow_dma_init_tx(DMA1_Channel2, TIM2, (&ow_buffer_tx[1]), 7);

    bus_error = OW_ERR_NONE;
    bus_status = OW_STS_SEND_IN_PROGRESS;

    OW_DMA_START(1, 2);
    ow_tim_start(TIM2);
}
