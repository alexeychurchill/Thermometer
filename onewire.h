#ifndef ONEWIRE_H
#define ONEWIRE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stm32f103xb.h>

#define OW_SINGLE_BYTE(value)   (const uint8_t[]) { (value) }, 1
#define OW_READ_SLOTS(count)    NULL, count

#define OW_CREATE_LINE( \
    tim_n,       \
    tim_out_n, \
    tim_in_n, \
    dma_in_n,       \
    dma_out_n,        \
    dma_channel_out_n, \
    dma_channel_in_n,   \
    tim_config_func) ow_init_set_peripherals( \
        TIM##tim_n,     \
        DMA##dma_in_n,     \
        DMA##dma_out_n##_Channel##dma_channel_out_n, \
        DMA##dma_in_n##_Channel##dma_channel_in_n   \
        TIM##tim_n##_IRQn,   \
        DMA##dma_in_n##dma_channel_in_n##_IRQn,      \
        tim_config_func);

typedef struct OwLine OwLine_t;

typedef enum OwError {
    OW_ERROR_NONE,
    OW_ERROR_NO_DEVICES,
    OW_ERROR_PRESENCE_WRONG_LENGTH
} OwError_t;

typedef void (*OwTimChannelConfigFunc_t)(TIM_TypeDef*);

// Initialization

OwLine_t ow_init_set_peripherals(
        TIM_TypeDef *tim,
        DMA_TypeDef *dma_in,
        DMA_Channel_TypeDef *dma_ch_out,
        DMA_Channel_TypeDef *dma_ch_in,
        IRQn_Type tim_irq,
        IRQn_Type dma_ch_in_irq,
        OwTimChannelConfigFunc_t configFunc,
        uint32_t *out_ccr,
        uint32_t *in_ccr,
        uint32_t flag_dma,
        uint32_t flag_isr,
        uint32_t dma_in_isr_reset
);

//void ow_init_set_peripherals(
//        OwLine_t *line,
//        TIM_TypeDef *tim,
//        DMA_TypeDef *dma_in,
//        DMA_Channel_TypeDef *dma_ch_out,
//        DMA_Channel_TypeDef *dma_ch_in
//);
//
//void ow_init_set_irq(OwLine_t *line, IRQn_Type tim_irq, IRQn_Type dma_ch_in_irq);
//
//void ow_init_set_tim_channel_config_func(OwLine_t *line, OwTimChannelConfigFunc_t configFunc);
//
//void ow_init_set_tim_ccr(OwLine_t *line, uint32_t *out_ccr, uint32_t *in_ccr);
//
//void ow_init_set_tim_in_dier_flags(
//        OwLine_t *line,
//        uint32_t flag_dma,
//        uint32_t flag_isr,
//        uint32_t dma_in_isr_reset
//);
//
//void ow_init_reset_status_flags(OwLine_t *line);

typedef struct OwTimCh {
    uint32_t* (*get_ccr)(TIM_TypeDef*);
    uint32_t* (*get_ccmr)(TIM_TypeDef*);
    uint32_t *dier_dma;
    uint32_t *dier_isr;
} OwTimCh_t;

void ow_init_set_tim(OwLine_t *line, const TIM_TypeDef *tim, IRQn_Type tim_irq);
void ow_init_set_tim_out();
void ow_init_set_tim_in();
void ow_init_set_out_dma_ch(OwLine_t *line, const DMA_Channel_TypeDef *ch);

// Operations

void ow_start();

void ow_reset();

uint32_t ow_is_busy();

OwError_t ow_get_error();

void ow_start_transceiver(uint16_t byte_len, bool wait_done);

// Pass [data] as NULL to put RX slots
void ow_txbuf_put(const uint8_t *data, uint32_t byte_len);

void ow_rxbuf_get(uint8_t *data, uint32_t byte_len);

#endif
