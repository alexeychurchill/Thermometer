
#include "stm32f1xx.h"
#include <stdint.h>
#include "uart.h"
#include "gpio.h"
#include "onewire.h"


volatile uint64_t sys_ticks = 0UL;

void delay_ms(const uint32_t ms);

void delay_us(const uint32_t us);

static volatile uint32_t stopwatch_time_start_us = 0;

void stopwatch_reset();

uint32_t stopwatch_time();

void SysTick_Handler(void) {
    sys_ticks++;
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
int main() {
    RCC -> CR |= RCC_CR_HSEON;
    while (!(RCC -> CR & RCC_CR_HSERDY)) ;

    RCC -> CFGR |= RCC_CFGR_PLLSRC;
    RCC -> CFGR |= RCC_CFGR_PLLMULL9;
    RCC -> CFGR |= RCC_CFGR_PPRE1_DIV2;
    RCC -> CFGR |= RCC_CFGR_PPRE2_DIV1;

    FLASH -> ACR |= FLASH_ACR_LATENCY_2;

    RCC -> CR |= RCC_CR_PLLON;
    while (!(RCC -> CR & RCC_CR_PLLRDY)) ;
    
    RCC -> CFGR |= RCC_CFGR_SW_PLL;
    while (!(RCC -> CFGR & RCC_CFGR_SWS_PLL)) ;

    SystemCoreClockUpdate();

    uint32_t tick_count = SystemCoreClock / 1000000;
    SysTick_Config(tick_count);

    RCC -> APB2ENR |= RCC_APB2ENR_IOPCEN;

    gpio_setup(GPIOC, 13, GPIO_OUT_PP, GPIO_MODE_OUT_50MHZ);

    uart1_init();

    RCC -> AHBENR |= RCC_AHBENR_DMA1EN;
    RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;
    gpio_setup(GPIOA, 0, GPIO_OUT_AF_OD, GPIO_MODE_OUT_50MHZ);

    ow_init();

    // Convert T

    ow_reset();

    while (ow_is_running()) ;

    ow_txbuf_put_bytes(OW_SINGLE_BYTE(0xCC));
    ow_start_transceiver(1);

    while (ow_is_running()) ;

    ow_txbuf_put_bytes(OW_SINGLE_BYTE(0x44));
    ow_start_transceiver(1);

    while (ow_is_running()) ;

    uint8_t done[] = { 0x00 };

    while (done[0] <= 0x00) {
        ow_txbuf_put_rx_slots(1);
        ow_start_transceiver(1);

        while (ow_is_running()) ;

        ow_rxbuf_get_bytes(done, 1);
    }

    // Read Scratchpad
    ow_reset();

    while (ow_is_running()) ;

    ow_txbuf_put_bytes(OW_SINGLE_BYTE(0xCC));
    ow_start_transceiver(1);

    while (ow_is_running()) ;

    ow_txbuf_put_bytes(OW_SINGLE_BYTE(0xBE));
    ow_start_transceiver(1);

    while (ow_is_running()) ;

    ow_txbuf_put_rx_slots(9);
    ow_start_transceiver(9);

    while (ow_is_running()) ;

    while (1) {
         gpio_reset(GPIOC, 13);
         delay_ms(125);
         gpio_set(GPIOC, 13);
         delay_ms(125);
    }

    return 0;
}
#pragma clang diagnostic pop


void delay_ms(const uint32_t ms) {
    delay_us(ms * 1000);
}

void delay_us(const uint32_t us) {
    uint64_t start_ticks = sys_ticks;
    while ((sys_ticks - start_ticks) < us) ;
}

void stopwatch_start() {
    stopwatch_time_start_us = sys_ticks;    
}

uint32_t stopwatch_time() {
    uint32_t time = sys_ticks - stopwatch_time_start_us;
    return time >= 0 ? time : 0;
}
