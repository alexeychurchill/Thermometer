
#include "stm32f1xx.h"
#include <stdint.h>
#include "uart.h"
#include "gpio.h"
#include "onewire.h"

#define GPIO_IN_MODE_FLOATING 0b01
#define GPIO_OUT_MODE_OD 0b01 // Open Drain

#define LED_PIN_RESET (~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13))
#define LED_PIN_CONFIG (GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0)


volatile uint64_t sys_ticks = 0UL;

void delay_ms(const uint32_t ms);

void delay_us(const uint32_t us);

static volatile uint32_t stopwatch_time_start_us = 0;

void stopwatch_reset();

uint32_t stopwatch_time();

void SysTick_Handler(void) {
    sys_ticks++;
}

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

    delay_ms(3000);

    one_wire_init();

    one_wire_reset();

    while (1) {
        gpio_reset(GPIOC, 13);
        delay_ms(125);
        gpio_set(GPIOC, 13);
        delay_ms(125);

        one_wire_status_t status = ow_status_get();
        one_wire_error_t err = ow_error_get();

        uart1_send_str("Status: ");
        uart1_send_number(status);
        uart1_send_str(" Error: ");
        uart1_send_number(err);
        uart1_send_str("\r\n");
    }

    return 0;
}


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
