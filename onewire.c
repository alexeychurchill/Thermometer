#include "onewire.h"
#include <stdint.h>
#include <stddef.h>
#include "stm32f1xx.h"
#include "utils.h"


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

#define OW_TIM_EVENTS_RESET_(line)        (TIM_DIER_UIE | (line) -> in_dier_flag_dma)
#define OW_TIM_EVENTS_RXTX_(line)         (TIM_DIER_UDE | (line) -> in_dier_flag_dma)
#define OW_TIM_EVENTS_WAIT_DEVICE_(line)  ((line) -> in_dier_flag_isr)

#define OW_BUF_DUMMY_INDEX(bytes_n) (bytes_n * UINT8_BIT_COUNT)
#define OW_BUF_PULSE_TO_BIT(p_len)  ((p_len) <= (OW_RX_LOW_THRESHOLD) ? (uint8_t)0x1 : (uint8_t)0x0)

#define OW_OP_GUARD()               { if (ow_busy) { return; }; ow_busy = 1; }
#define OW_OP_GUARD_SOFT()          if (ow_busy) { return; }
#define OW_OP_DONE()                ow_busy = 0;

#define OW_OP_GUARD_(ow_line)       { if ((ow_line) -> is_busy) { return; }; (ow_line) -> is_busy = true; }
#define OW_OP_GUARD_SOFT_(ow_line)  if ((ow_line) -> is_busy) { return; }
#define OW_OP_DONE_(ow_line)        (ow_line) -> is_busy = false;


// Flag which indicates running operation
static volatile uint32_t ow_busy;
static bool ow_is_reseting;
static bool ow_wait_done_device;

static OwError_t ow_error;

static uint16_t ow_buffer_tx[OW_BUFFER_TX_LEN];
static uint16_t ow_buffer_rx[OW_BUFFER_RX_LEN];

// 1-Wire line structure

struct OwLine {
    // Peripherals

    // Peripherals ptrs

    TIM_TypeDef *tim;
    DMA_TypeDef *dma_in;
    DMA_Channel_TypeDef *dma_ch_out;
    DMA_Channel_TypeDef *dma_ch_in;

    // Peripherals interrupts

    IRQn_Type tim_irq;
    IRQn_Type dma_ch_in_irq;

    // Timer

    OwTimChannelConfigFunc_t tim_config_channels;

    uint32_t *out_ccr;
    uint32_t *in_ccr;

    uint32_t in_dier_flag_dma;
    uint32_t in_dier_flag_isr;

    // DMA

    uint32_t dma_in_isr_reset;

    // 1-Wire data

    volatile bool is_busy;
    bool wait_devices_free;

    bool reset_running;

    OwError_t error;

    uint16_t buffer_tx[OW_BUFFER_TX_LEN];
    uint16_t buffer_rx[OW_BUFFER_RX_LEN];
};

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

void ow_tim_start2__(OwLine_t *line) {
    TIM_TypeDef *tim = line -> tim;
    tim -> CR1 = TIM_CR1_ARPE;
    tim -> DIER = 0x0;
//    tim -> CCMR1 = (TIM_CH1_OUT_PWM1 | TIM_CH2_IN_TI1);
//    tim -> CCER = (TIM_CH1_EN_INV | TIM_CH2_EN);
    line -> tim_config_channels(tim);
    tim -> PSC = OW_TIM_PSC;
    tim -> ARR = OW_SLOT_LEN;
    *(line -> out_ccr) = OW_TX_DUMMY;
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

static FORCE_INLINE void ow_dma_init_tx2__(OwLine_t *line, uint16_t bit_len) {
    DMA_Channel_TypeDef *dma = line -> dma_ch_out;
    dma -> CCR = OW_DMA_CCR_TX;
    dma -> CPAR = (uint32_t)(line -> out_ccr);
    dma -> CMAR = (uint32_t)(line -> buffer_tx);
    dma -> CNDTR = bit_len;
}

static FORCE_INLINE void ow_dma_init_rx__(DMA_Channel_TypeDef *dma, TIM_TypeDef *tim, uint16_t *buf, uint16_t bit_len) {
    dma -> CCR = OW_DMA_CCR_RX;
    dma -> CPAR = (uint32_t)(&(tim -> CCR2));
    dma -> CMAR = (uint32_t)(buf);
    dma -> CNDTR = bit_len;
}

static FORCE_INLINE void ow_dma_init_rx2__(OwLine_t *line, uint16_t bit_len) {
    DMA_Channel_TypeDef *dma = line -> dma_ch_in;
    dma -> CCR = OW_DMA_CCR_RX;
    dma -> CPAR = (uint32_t)(line -> in_ccr);
    dma -> CMAR = (uint32_t)(line -> buffer_rx);
    dma -> CNDTR = bit_len;
}

// TX Buffer common procedures

static FORCE_INLINE void ow_txbuf_put_byte__(uint8_t byte, uint32_t buffer_offset) {
    for (uint32_t bit_n = 0; bit_n < UINT8_BIT_COUNT; ++bit_n) {
        ow_buffer_tx[buffer_offset + bit_n] = (UINT8_BIT_VALUE(byte, bit_n)) ? OW_TX_HIGH_LEN : OW_TX_LOW_LEN;
    }
}

static FORCE_INLINE void ow_txbuf_put_byte2__(OwLine_t *line, uint8_t byte, uint32_t buffer_offset) {
    for (uint32_t bit_n = 0; bit_n < UINT8_BIT_COUNT; ++bit_n) {
        (line -> buffer_tx)[buffer_offset + bit_n] = (UINT8_BIT_VALUE(byte, bit_n)) ? OW_TX_HIGH_LEN : OW_TX_LOW_LEN;
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

static FORCE_INLINE uint8_t ow_rxbuf_get_byte2__(OwLine_t *line, uint32_t bit_offset) {
    uint8_t byte = 0x0;

    for (uint32_t bit_n = 0; bit_n < UINT8_BIT_COUNT; ++bit_n) {
        uint8_t bit_value = OW_BUF_PULSE_TO_BIT((line -> buffer_rx)[bit_offset + bit_n]);
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

static void FORCE_INLINE ow_check_presence_length2__(OwLine_t *line) {
    uint16_t presence_len = (line -> buffer_rx[1]) - (line -> buffer_tx[0]);
    bool presence_too_short = presence_len < OW_RESET_ACK_LEN_MIN;
    bool presence_too_long = presence_len > OW_RESET_ACK_LEN_MAX;
    if (presence_too_short || presence_too_long) {
        line -> error = OW_ERROR_PRESENCE_WRONG_LENGTH;
    }
}

// Timer TX pulse management

static FORCE_INLINE void ow_tim_set_pulse_force__(TIM_TypeDef *tim, uint32_t pulse_len, uint32_t low_len) {
    tim -> ARR = pulse_len;
    tim -> CCR1 = low_len;

    tim -> EGR = TIM_EGR_UG;
    tim -> SR = 0x0;
}

static FORCE_INLINE void ow_tim_set_pulse_force2__(OwLine_t *line, uint32_t pulse_len, uint32_t low_len) {
    TIM_TypeDef *tim = line -> tim;
    tim -> ARR = pulse_len;
    *(line -> out_ccr) = low_len;

    tim -> EGR = TIM_EGR_UG;
    tim -> SR = 0x0;
}

static FORCE_INLINE void ow_tim_set_pulse__(TIM_TypeDef *tim, uint32_t pulse_len, uint32_t low_len) {
    tim -> ARR = pulse_len;
    tim -> CCR1 = low_len;
}

static FORCE_INLINE void ow_tim_set_pulse2__(OwLine_t *line, uint32_t pulse_len, uint32_t low_len) {
    TIM_TypeDef *tim = line -> tim;
    tim -> ARR = pulse_len;
    *(line -> out_ccr) = low_len;
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

void ow_isr_tim(OwLine_t *line) {
    TIM_TypeDef *tim = line -> tim;
    if ((tim -> SR & TIM_SR_CC2IF) && (line -> wait_devices_free)) {
        if (*(line -> in_ccr) < OW_RX_LOW_THRESHOLD) {
            tim -> DIER = OW_TIM_EVENTS_DISABLE;
            line -> wait_devices_free = false;
            OW_OP_DONE_(line);
        }
        line -> tim -> SR = 0x0;
        return;
    }

    if (tim-> SR & TIM_SR_UIF) {
        tim -> DIER = OW_TIM_EVENTS_DISABLE;
        line -> error = OW_ERROR_NO_DEVICES;
        OW_OP_DONE_(line);
        tim -> SR = 0x0;
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

void ow_isr_dma_ch_in(OwLine_t *line) {
    line -> tim -> DIER = OW_TIM_EVENTS_DISABLE;

    ow_tim_set_pulse_force2__(line, OW_SLOT_LEN, OW_TX_DUMMY);

    if (line -> reset_running) {
        ow_check_presence_length2__(line);
        line -> reset_running = false;
    }

    if (line -> wait_devices_free) {
        ow_tim_set_pulse2__(line, OW_SLOT_LEN, OW_RX_SLOT_LEN);
        line -> tim -> DIER = OW_TIM_EVENTS_WAIT_DEVICE_(line);
    } else {
        OW_OP_DONE_(line);
    }

    line -> dma_in -> IFCR = line -> dma_in_isr_reset;
}

// 1-Wire procedures

// Initialization procedures

OwLine_t ow_init_set_peripherals(
        TIM_TypeDef *tim,
        DMA_TypeDef *dma_in,
        DMA_Channel_TypeDef *dma_ch_out,
        DMA_Channel_TypeDef *dma_ch_in,
        const IRQn_Type tim_irq,
        const IRQn_Type dma_ch_in_irq,
        const OwTimChannelConfigFunc_t configFunc,
        uint32_t *out_ccr,
        uint32_t *in_ccr,
        const uint32_t flag_dma,
        const uint32_t flag_isr,
        const uint32_t dma_in_isr_reset
) {
//    line -> tim = tim;
//
//    line -> dma_in = dma_in;
//    line -> dma_ch_out = dma_ch_out;
//    line -> dma_ch_in = dma_ch_in;
//
//    line -> tim_irq = tim_irq;
//    line -> dma_ch_in_irq = dma_ch_in_irq;
//
//    line -> tim_config_channels = configFunc;
//
//    line -> out_ccr = out_ccr;
//    line -> in_ccr = in_ccr;
//
//    line -> in_dier_flag_dma = flag_dma;
//    line -> in_dier_flag_isr = flag_isr;
//    line -> dma_in_isr_reset = dma_in_isr_reset;
//
//    line -> is_busy = false;
//    line -> wait_devices_free = false;
//    line -> error = OW_ERROR_NONE;
//    line -> reset_running = false;

    return (OwLine_t) {
            .tim = tim,

            .dma_in = dma_in,
            .dma_ch_out = dma_ch_out,
            .dma_ch_in = dma_ch_in,

            .tim_irq = tim_irq,
            .dma_ch_in_irq = dma_ch_in_irq,

            .tim_config_channels = configFunc,

            .out_ccr = out_ccr,
            .in_ccr = in_ccr,

            .in_dier_flag_dma = flag_dma,
            .in_dier_flag_isr = flag_isr,
            .dma_in_isr_reset = dma_in_isr_reset,

            .is_busy = false,
            .wait_devices_free = false,
            .error = OW_ERROR_NONE,
            .reset_running = false,
    };
}

//void ow_init_set_irq(OwLine_t *line, const IRQn_Type tim_irq, const IRQn_Type dma_ch_in_irq) {
//    line -> tim_irq = tim_irq;
//    line -> dma_ch_in_irq = dma_ch_in_irq;
//}

//void ow_init_set_tim_channel_config_func(OwLine_t *line, const OwTimChannelConfigFunc_t configFunc) {
//    line -> tim_config_channels = configFunc;
//}

//void ow_init_set_tim_ccr(OwLine_t *line, uint32_t *out_ccr, uint32_t *in_ccr) {
//    line -> out_ccr = out_ccr;
//    line -> in_ccr = in_ccr;
//}

//void ow_init_set_tim_in_dier_flags(
//        OwLine_t *line,
//        const uint32_t flag_dma,
//        const uint32_t flag_isr,
//        const uint32_t dma_in_isr_reset
//) {
//    line -> in_dier_flag_dma = flag_dma;
//    line -> in_dier_flag_isr = flag_isr;
//    line -> dma_in_isr_reset = dma_in_isr_reset;
//}

//void ow_init_reset_status_flags(OwLine_t *line) {
//    line -> is_busy = false;
//    line -> wait_devices_free = false;
//    line -> error = OW_ERROR_NONE;
//    line -> reset_running = false;
//}

// Status procedures

uint32_t ow_is_busy() {
    return ow_busy ? 1 : 0;
}

bool ow_is_busy_(OwLine_t *line) {
    return (line -> is_busy) ? true : false;
}

OwError_t ow_get_error() {
    return ow_error;
}

OwError_t ow_get_error_(OwLine_t *line) {
    return line -> error;
}

// Start procedures

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

bool ow_start_(OwLine_t *line) {
    if (line -> tim == NULL) {
        return false;
    }

    if ((line -> out_ccr == NULL) || (line -> in_ccr == NULL)) {
        return false;
    }

    if ((line -> dma_in == NULL) || (line -> dma_ch_out == NULL) || (line -> dma_ch_in == NULL)) {
        return false;
    }

    ow_tim_start2__(line);

    NVIC_EnableIRQ(line -> tim_irq);
    NVIC_EnableIRQ(line -> dma_ch_in_irq);

    return true;
}

// Rx/Tx Procedures

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

void ow_reset_(OwLine_t *line) {
    OW_OP_GUARD_(line);

    line -> reset_running = true;
    line -> wait_devices_free = false;

    ow_dma_init_rx2__(line, 2);

    ow_tim_set_pulse_force2__(line, OW_RESET_SLOT_LEN, OW_RESET_PULSE_LEN);
    ow_tim_set_pulse2__(line, OW_SLOT_LEN, OW_TX_DUMMY);
    line -> tim -> DIER = OW_TIM_EVENTS_RESET_(line);

    line -> dma_ch_in -> CCR |= DMA_CCR_EN;
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

void ow_start_transceiver_(OwLine_t *line, uint16_t byte_len, bool wait_done) {
    OW_OP_GUARD_(line);

    line -> wait_devices_free = wait_done;

    uint32_t bit_len = byte_len * UINT8_BIT_COUNT;
    ow_dma_init_tx2__(line, bit_len + 1); // +1 DUMMY bit
    ow_dma_init_rx2__(line, bit_len);

    line -> dma_ch_out -> CCR |= DMA_CCR_EN;
    line -> dma_ch_in -> CCR |= DMA_CCR_EN;

    line -> tim -> DIER = OW_TIM_EVENTS_RXTX_(line);
}

// Buffer procedures

void ow_txbuf_put(const uint8_t *data, uint32_t byte_len) {
    OW_OP_GUARD_SOFT();

    uint32_t dummy_bit_index = OW_BUF_DUMMY_INDEX(byte_len);
    if (dummy_bit_index >= OW_BUFFER_TX_LEN) {
        return;
    }

    if (data == NULL) {
        for (uint32_t bit_n = 0; bit_n < byte_len * UINT8_BIT_COUNT; bit_n++) {
            ow_buffer_tx[bit_n] = OW_RX_SLOT_LEN;
        }
        ow_buffer_tx[dummy_bit_index] = OW_TX_DUMMY;
        return;
    }

    for (uint32_t byte_n = 0; byte_n < byte_len; ++byte_n) {
        ow_txbuf_put_byte__(data[byte_n], byte_n * UINT8_BIT_COUNT);
    }

    ow_buffer_tx[dummy_bit_index] = OW_TX_DUMMY;
}

void ow_txbuf_put_(OwLine_t *line, const uint8_t *data, uint32_t byte_len) {
    OW_OP_GUARD_SOFT_(line);

    uint32_t dummy_bit_index = OW_BUF_DUMMY_INDEX(byte_len);
    if (dummy_bit_index >= OW_BUFFER_TX_LEN) {
        return;
    }

    if (data == NULL) {
        for (uint32_t bit_n = 0; bit_n < byte_len * UINT8_BIT_COUNT; bit_n++) {
            (line -> buffer_tx)[bit_n] = OW_RX_SLOT_LEN;
        }
        (line -> buffer_tx)[dummy_bit_index] = OW_TX_DUMMY;
        return;
    }

    for (uint32_t byte_n = 0; byte_n < byte_len; ++byte_n) {
        ow_txbuf_put_byte2__(line, data[byte_n], byte_n * UINT8_BIT_COUNT);
    }

    (line -> buffer_tx)[dummy_bit_index] = OW_TX_DUMMY;
}

void ow_rxbuf_get(uint8_t *data, uint32_t byte_len) {
    for (uint32_t byte_n = 0; byte_n < byte_len; ++byte_n) {
        data[byte_n] = ow_rxbuf_get_byte__(byte_n * UINT8_BIT_COUNT);
    }
}

void ow_rxbuf_get_(OwLine_t *line, uint8_t *data, uint32_t byte_len) {
    for (uint32_t byte_n = 0; byte_n < byte_len; ++byte_n) {
        data[byte_n] = ow_rxbuf_get_byte2__(line, byte_n * UINT8_BIT_COUNT);
    }
}
