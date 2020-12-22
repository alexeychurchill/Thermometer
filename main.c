#include "stm32f1xx.h"
#include "config.h"
#include "rcc_setup.h"
#include "uart.h"
#include "gpio.h"
#include "onewire.h"
#include "onewire_stm32.h"
#include "poll_timer.h"
#include "temp_sensor_dispatcher.h"
#include "./ui/ui_screen_temp.h"
#include "display.h"


static const UiDisplay_t display = {
        .clear = display_buffer_clear,
        .put_pixel = display_buffer_put_pixel,
        .text_set_page = display_text_set_page,
        .text_set_offset_x = display_text_set_offset_x,
        .text_set_align = display_text_set_align,
        .text_set_font = display_text_set_font,
        .put_text = display_buffer_put_text,
};

static const UiScreen_t screen_temp = {
        .draw = ui_screen_temp_draw,
};

static const OwBusLine_t ow_line = {
        .is_busy = ow_is_busy,
        .get_error = ow_get_error,
        .send_reset = ow_send_reset,
        .get_operation = ow_get_operation,
        .put_tx_buffer = ow_txbuf_put,
        .get_rx_buffer = ow_rxbuf_get,
        .start_rxtx = ow_start_rxtx,
};


int main() {
    rcc_setup_clocking();
    rcc_enable_peripherals_clocking();

    gpio_setup(GPIOC, 13, GPIO_OUT_PP, GPIO_MODE_OUT_50MHZ); // DevBoard LED

    uart1_init();
    gpio_setup(GPIOA, 0, GPIO_OUT_AF_OD, GPIO_MODE_OUT_50MHZ); // 1-Wire Timer CH1/2 GPIO
    ow_start_bus();
    poll_timer_init(POLL_TIMER);

    tsd_init(&ow_line);

    gpio_reset(GPIOC, 13);

    gpio_setup(GPIOB, 10, GPIO_OUT_AF_OD, GPIO_MODE_OUT_50MHZ); // Display I2C
    gpio_setup(GPIOB, 11, GPIO_OUT_AF_OD, GPIO_MODE_OUT_50MHZ); // Display I2C

    display_init();
    display_buffer_clear();
    display_flush();

    while (true) {
        tsd_dispatch_state();
        screen_temp.draw(&display);
        display_flush();
    }

    return 0;
}
