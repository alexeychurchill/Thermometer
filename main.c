
#include "stm32f1xx.h"
#include <stdint.h>
#include "uart.h"

#define GPIO_IN_MODE_FLOATING 0b01
#define GPIO_OUT_MODE_OD 0b01

#define LED_PIN_RESET (~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13))
#define LED_PIN_CONFIG (GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0)



volatile uint32_t sys_ticks = 0;

void delay(const uint32_t ms);

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

    uint32_t tick_count = SystemCoreClock / 1000;
    SysTick_Config(tick_count);

    RCC -> APB2ENR |= RCC_APB2ENR_IOPCEN;

    GPIOC -> CRH &= LED_PIN_RESET;
    GPIOC -> CRH |= LED_PIN_CONFIG;

    uart1_init();

    while (1) {
        GPIOC -> BSRR = GPIO_BSRR_BS13;
        delay_ms(3000);
        GPIOC -> BSRR = GPIO_BSRR_BR13;
        delay_ms(3000);
        uart1_send_strn("Test uart!");
        uart1_send_strn("А тепер Українською! :)");
    }

    return 0;
}


void delay_ms(const uint32_t ms) {
    uint32_t start_ticks = sys_ticks;
    while ((sys_ticks - start_ticks) < ms) ;
}
