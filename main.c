
#include "stm32f1xx.h"
#include <stdint.h>
#include "uart.h"
#include "gpio.h"
#include "onewire.h"
#include "onewire_stm32.h"
#include "ds18b20.h"


volatile uint64_t sys_ticks = 0UL;

void delay_ms(const uint32_t ms);

void delay_us(const uint32_t us);

static volatile uint32_t stopwatch_time_start_us = 0;

void stopwatch_reset();

uint32_t stopwatch_time();

void SysTick_Handler(void) {
    sys_ticks++;
}

static const OwBusLine_t ow_line = {
        .is_busy = ow_is_busy,
        .get_error = ow_get_error,
        .put_tx_buffer = ow_txbuf_put,
        .get_rx_buffer = ow_rxbuf_get,
        .start_rxtx = ow_start_rxtx,
};

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

    ow_start_bus();

    DS18B20Sensor_t temp_sensor;

    while (1) {
         gpio_reset(GPIOC, 13);
         delay_ms(125);
         gpio_set(GPIOC, 13);
         delay_ms(125);

         ds18b20_convert_t_send(&ow_line);
         while (ow_line.is_busy()) ;

         ds18b20_send_read_scratchpad(&ow_line, &temp_sensor);
         while (ow_line.is_busy()) ;

         // TODO: Send temp to the UART
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
