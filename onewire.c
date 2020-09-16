#include "onewire.h"
#include <stdint.h>
#include "stm32f1xx.h"
#include "gpio.h"


/**
 * TODO: Move all HW-specific things to the structures (such as pointers to DMA channels structs)
 * TODO: Call all ISRs from outside:
 * ISRs should just call 1-Wire module procedures passing aforementioned 1-Wire structures
 */


#define OW_TIM_PSC                  71      // 72

#define OW_RESET_SLOT_LEN           2999    // 3000 us
#define OW_RESET_PULSE_LEN          499     // 500 us
#define OW_RESET_ACK_LEN_MIN        74      // 75 us
#define OW_RESET_ACK_LEN_MAX        299     // 300 us

#define OW_BUFFER_RESET_LEN         2        // 4 bytes: RESET end time and PRESENCE end time
#define OW_BUFFER_TX_LEN            129      // 11 bytes: 11 * 8 bit data + 1 dummy bit
#define OW_BUFFER_RX_LEN            128      // 11 bytes: 11 * 8 bit data

#define OW_SLOT_LEN                 ((uint16_t) 70)      // 70 us
#define OW_TX_DUMMY                 ((uint16_t) 0)       // 0 us
#define OW_TX_LOW_LEN               ((uint16_t) 65)      // 65 us
#define OW_TX_HIGH_LEN              ((uint16_t) 13)      // 13 us

#define OW_RX_SLOT_LEN              ((uint16_t) 13)      // 13 us
#define OW_RX_LOW_THRESHOLD         ((uint16_t) 15)      // 15 us

#define TIM_CH1_OUT_PWM1            (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE)
#define TIM_CH2_IN_TI1              (TIM_CCMR1_CC2S_1)
#define TIM_CH1_EN_INV              (TIM_CCER_CC1E | TIM_CCER_CC1P)
#define TIM_CH2_EN                  (TIM_CCER_CC2E)

#define OW_DMA_CCR_BASE             (DMA_CCR_MINC | DMA_CCR_PSIZE_0 | DMA_CCR_MSIZE_0)
#define OW_DMA_CCR_TX               (OW_DMA_CCR_BASE | DMA_CCR_DIR)
#define OW_DMA_CCR_RX               (OW_DMA_CCR_BASE | DMA_CCR_TCIE)

#define OW_TIM_EVENTS_DISABLE       (0x0)
#define OW_TIM_EVENTS_RESET         (TIM_DIER_UIE | TIM_DIER_CC2DE)
#define OW_TIM_EVENTS_RXTX          (TIM_DIER_UDE | TIM_DIER_CC2DE)
#define OW_TIM_EVENTS_WAIT_DEV      (TIM_DIER_CC2IE)

#define OW_BUF_DUMMY_INDEX(bytes_n) (bytes_n * UINT8_BIT_COUNT)
#define OW_BUF_PULSE_TO_BIT(p_len)  ((p_len) <= (OW_RX_LOW_THRESHOLD) ? (uint8_t)0x1 : (uint8_t)0x0)

#define OW_OP_GUARD()               { if (ow_busy) { return; }; ow_busy = 1; }
#define OW_OP_GUARD_SOFT()          if (ow_busy) { return; }
#define OW_OP_DONE()                ow_busy = 0;


// Flag which indicates running operation
static volatile uint32_t ow_busy;
static bool ow_is_reseting;
static bool ow_wait_done_device;

static OwError_t ow_error;

static uint16_t ow_buffer_tx[OW_BUFFER_TX_LEN];
static uint16_t ow_buffer_rx[OW_BUFFER_RX_LEN];


// Timer Configuration procedures

void ow_tim_start__(TIM_TypeDef *tim) {
    tim -> CR1 = TIM_CR1_ARPE;
    tim -> DIER = 0x0;
    tim -> CCMR1 = (TIM_CH1_OUT_PWM1 | TIM_CH2_IN_TI1);
    tim -> CCER = (TIM_CH1_EN_INV | TIM_CH2_EN);
    tim -> PSC = OW_TIM_PSC;
    tim -> ARR = OW_SLOT_LEN;
    tim -> CCR1 = OW_TX_DUMMY;
    tim -> SR = 0x0;
    tim -> CR1 |= TIM_CR1_CEN;
}

// DMA Configuration procedures

static FORCE_INLINE void ow_dma_init_tx__(DMA_Channel_TypeDef *dma, TIM_TypeDef *tim, uint16_t *buf, uint16_t bit_len) {
    dma -> CCR = OW_DMA_CCR_TX;
    dma -> CPAR = (uint32_t)(&(tim -> CCR1));
    dma -> CMAR = (uint32_t)(buf);
    dma -> CNDTR = bit_len;
}

static FORCE_INLINE void ow_dma_init_rx__(DMA_Channel_TypeDef *dma, TIM_TypeDef *tim, uint16_t *buf, uint16_t bit_len) {
    dma -> CCR = OW_DMA_CCR_RX;
    dma -> CPAR = (uint32_t)(&(tim -> CCR2));
    dma -> CMAR = (uint32_t)(buf);
    dma -> CNDTR = bit_len;
}

// TX Buffer common procedures

static FORCE_INLINE void ow_txbuf_put_byte__(uint8_t byte, uint32_t buffer_offset) {
    for (uint32_t bit_n = 0; bit_n < UINT8_BIT_COUNT; ++bit_n) {
        ow_buffer_tx[buffer_offset + bit_n] = (UINT8_BIT_VALUE(byte, bit_n)) ? OW_TX_HIGH_LEN : OW_TX_LOW_LEN;
    }
}

static FORCE_INLINE void ow_txbuf_put_rx_slots__(uint32_t bit_count, uint8_t buffer_offset) {
    for (uint32_t bit_n = 0; bit_n < bit_count; ++bit_n) {
        ow_buffer_tx[buffer_offset + bit_n] = OW_RX_SLOT_LEN;
    }
}

// RX Buffer common procedures

static FORCE_INLINE uint8_t ow_rxbuf_get_byte__(uint32_t bit_offset) {
    uint8_t byte = 0x0;

    for (uint32_t bit_n = 0; bit_n < UINT8_BIT_COUNT; ++bit_n) {
        uint8_t bit_value = OW_BUF_PULSE_TO_BIT(ow_buffer_rx[bit_offset + bit_n]);
        byte |= (bit_value << bit_n);
    }

    return byte;
}

// Presence length check

static void FORCE_INLINE ow_check_presence_length__() {
    uint16_t presence_len = ow_buffer_rx[1] - ow_buffer_tx[0];
    bool presence_too_short = presence_len < OW_RESET_ACK_LEN_MIN;
    bool presence_too_long = presence_len > OW_RESET_ACK_LEN_MAX;
    if (presence_too_short || presence_too_long) {
        ow_error = OW_ERROR_PRESENCE_WRONG_LENGTH;
    }
}

// Timer TX pulse management

static FORCE_INLINE void ow_tim_set_pulse_force__(TIM_TypeDef *tim, uint32_t pulse_len, uint32_t low_len) {
    tim -> ARR = pulse_len;
    tim -> CCR1 = low_len;

    tim -> EGR = TIM_EGR_UG;
    tim -> SR = 0x0;
}

static FORCE_INLINE void ow_tim_set_pulse__(TIM_TypeDef *tim, uint32_t pulse_len, uint32_t low_len) {
    tim -> ARR = pulse_len;
    tim -> CCR1 = low_len;
}

// ISRs

void TIM2_IRQHandler(void) {
    if ((TIM2 -> SR & TIM_SR_CC2IF) && ow_wait_done_device) {
        if (TIM2 -> CCR2 < OW_RX_LOW_THRESHOLD) {
            TIM2 -> DIER = OW_TIM_EVENTS_DISABLE;
            ow_wait_done_device = false;
            OW_OP_DONE();
        }
        TIM2 -> SR = 0x0;
        return;
    }

    if (TIM2 -> SR & TIM_SR_UIF) {
        TIM2->DIER = OW_TIM_EVENTS_DISABLE;
        ow_error = OW_ERROR_NO_DEVICES;
        OW_OP_DONE();
        TIM2 -> SR = 0x0;
        return;
    }
}

void DMA1_Channel7_IRQHandler(void) {
    TIM2 -> DIER = OW_TIM_EVENTS_DISABLE;

    ow_tim_set_pulse_force__(TIM2, OW_SLOT_LEN, OW_TX_DUMMY);

    if (ow_is_reseting) {
        ow_check_presence_length__();
        ow_is_reseting = false;
    }

    if (ow_wait_done_device) {
        ow_tim_set_pulse__(TIM2, OW_SLOT_LEN, OW_RX_SLOT_LEN);
        TIM2 -> DIER = OW_TIM_EVENTS_WAIT_DEV;
    } else {
        OW_OP_DONE();
    }

    DMA1 -> IFCR = DMA_IFCR_CTCIF7;
}

// 1-Wire procedures

uint32_t ow_is_busy() {
    return ow_busy ? 1 : 0;
}

OwError_t ow_get_error() {
    return ow_error;
}

void ow_start() {
    ow_busy = 1;
    ow_error = OW_ERROR_NONE;
    ow_wait_done_device = false;

    ow_is_reseting = false;

    ow_tim_start__(TIM2);

    NVIC_EnableIRQ(DMA1_Channel7_IRQn);
    NVIC_EnableIRQ(TIM2_IRQn);

    ow_busy = 0;
}

void ow_reset() {
    OW_OP_GUARD();

    ow_is_reseting = true;
    ow_wait_done_device = false;

    ow_dma_init_rx__(DMA1_Channel7, TIM2, (&ow_buffer_rx[0]), 2);

    ow_tim_set_pulse_force__(TIM2, OW_RESET_SLOT_LEN, OW_RESET_PULSE_LEN);

    ow_tim_set_pulse__(TIM2, OW_SLOT_LEN, OW_TX_DUMMY);

    TIM2 -> DIER = OW_TIM_EVENTS_RESET;

    DMA1_Channel7 -> CCR |= DMA_CCR_EN;
}

void ow_start_transceiver(uint16_t byte_len, bool wait_done) {
    OW_OP_GUARD();

    ow_wait_done_device = wait_done;

    uint32_t bit_len = byte_len * UINT8_BIT_COUNT;
    ow_dma_init_tx__(DMA1_Channel2, TIM2, (&ow_buffer_tx[0]), bit_len + 1); // +1 DUMMY bit
    ow_dma_init_rx__(DMA1_Channel7, TIM2, (&ow_buffer_rx[0]), bit_len);

    DMA1_Channel2 -> CCR |= DMA_CCR_EN;
    DMA1_Channel7 -> CCR |= DMA_CCR_EN;

    TIM2 -> DIER = OW_TIM_EVENTS_RXTX;
}

void ow_txbuf_put_bytes(const uint8_t *data, uint32_t byte_len) {
    OW_OP_GUARD_SOFT();

    uint32_t dummy_bit_index = OW_BUF_DUMMY_INDEX(byte_len);
    if (dummy_bit_index >= OW_BUFFER_TX_LEN) {
        return;
    }

    for (int byte_n = 0; byte_n < byte_len; ++byte_n) {
        ow_txbuf_put_byte__(data[byte_n], byte_n * UINT8_BIT_COUNT);
    }

    ow_buffer_tx[dummy_bit_index] = OW_TX_DUMMY;
}

void ow_txbuf_put_rx_slots(uint32_t byte_len) {
    OW_OP_GUARD_SOFT();

    uint32_t dummy_bit_index = OW_BUF_DUMMY_INDEX(byte_len);
    if (dummy_bit_index >= OW_BUFFER_TX_LEN) {
        return;
    }

    ow_txbuf_put_rx_slots__(byte_len * UINT8_BIT_COUNT, 0);

    ow_buffer_tx[dummy_bit_index] = OW_TX_DUMMY;
}

void ow_rxbuf_get_bytes(uint8_t *data, uint32_t byte_len) {
    for (uint32_t byte_n = 0; byte_n < byte_len; ++byte_n) {
        data[byte_n] = ow_rxbuf_get_byte__(byte_n * UINT8_BIT_COUNT);
    }
}
