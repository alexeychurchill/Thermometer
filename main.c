
#include "stm32f1xx.h"
#include <stdint.h>
#include "uart.h"
#include "gpio.h"
#include "onewire.h"

#define GPIO_IN_MODE_FLOATING 0b01
#define GPIO_OUT_MODE_OD 0b01 // Open Drain

#define LED_PIN_RESET (~(GPIO_CRH_MODE13 | GPIO_CRH_CNF13))
#define LED_PIN_CONFIG (GPIO_CRH_MODE13_1 | GPIO_CRH_MODE13_0)

typedef enum DS18B20_State {
    DS_NONE = 0,
    DS_RESET, 
    DS_SKIP_ROM, 
    DS_CONVERT_T, 
    DS_CONVERT_T_READ_STATUS
} DS18B20_State_t;

static volatile DS18B20_State_t ds_state = DS_NONE; 


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

    RCC -> AHBENR |= RCC_AHBENR_DMA1EN;

    one_wire_init();

    ds_state = DS_NONE;

    while (1) {
        // gpio_reset(GPIOC, 13);
        // delay_ms(125);
        // gpio_set(GPIOC, 13);
        // delay_ms(125);

        if (ds_state == DS_NONE && ow_status_get() == OW_STS_IDLE) {
            ds_state = DS_RESET;
            one_wire_reset();
        } else if (ds_state == DS_RESET && ow_status_get() == OW_STS_RESET_DONE && ow_error_get() == OW_ERR_NONE) {
            ds_state = DS_SKIP_ROM;
            ow_tx_byte(0xCC);
        } else if (ds_state == DS_SKIP_ROM && ow_status_get() == OW_STS_SEND_DONE && ow_error_get() == OW_ERR_NONE) {
            ds_state = DS_CONVERT_T; 
            ow_tx_byte(0x44);
        } else if (ds_state == DS_CONVERT_T && ow_status_get() == OW_STS_SEND_DONE && ow_error_get() == OW_ERR_NONE) {
            ow_tx_byte(0xFF);
        }
        
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
