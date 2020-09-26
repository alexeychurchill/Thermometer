
#include "stm32f1xx.h"
#include <stdint.h>
#include "rcc_setup.h"
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

//static const OwBusLine_t ow_line = {
//        .is_busy = ow_is_busy,
//        .get_error = ow_get_error,
//        .put_tx_buffer = ow_txbuf_put,
//        .get_rx_buffer = ow_rxbuf_get,
//        .start_rxtx = ow_start_rxtx,
//};

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
int main() {
    rcc_setup_clocking();
    rcc_enable_peripherals_clocking();


    gpio_setup(GPIOC, 13, GPIO_OUT_PP, GPIO_MODE_OUT_50MHZ);

    uart1_init();
    gpio_setup(GPIOA, 0, GPIO_OUT_AF_OD, GPIO_MODE_OUT_50MHZ);

    ow_start_bus();

//    DS18B20Sensor_t temp_sensor;

    uint8_t ow_wait_buf[1];

    while (1) {
         gpio_reset(GPIOC, 13);
         delay_ms(125);
         gpio_set(GPIOC, 13);
         delay_ms(125);

//         ds18b20_convert_t_send(&ow_line);
//         while (ow_line.is_busy()) ;
//
//         ds18b20_send_read_scratchpad(&ow_line, &temp_sensor);
//         while (ow_line.is_busy()) ;

        while (ow_is_busy()) ;

        ow_send_reset();

        while (ow_is_busy()) ;

        ow_txbuf_put(OW_SINGLE_BYTE(0xCC), false);
        ow_txbuf_put(OW_SINGLE_BYTE(0x44), true);

        ow_start_rxtx();

        while (ow_is_busy()) ;

        ow_txbuf_put(OW_SINGLE_BYTE(0xFF), false);

        bool ow_converting = true;
        while (ow_converting) {
            ow_start_rxtx();

            while (ow_is_busy()) ;

            ow_rxbuf_get(ow_wait_buf, 1);
            if (ow_wait_buf[0] != 0x0) {
                ow_converting = false;
                break;
            }
        }

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
