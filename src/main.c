#include <stdbool.h>
#include "stm32f1xx.h"
#include "config.h"
#include "rcc_setup.h"
#include "interfaces/uart.h"
#include "gpio.h"
#include "flash_settings.h"
#include "interfaces/onewire.h"
#include "interfaces/onewire_stm32.h"
#include "interfaces/i2c.h"
#include "poll_timer.h"
#include "temp_sensor_dispatcher.h"
#include "screens/ui_screen_temp.h"
#include "display.h"
#include "interfaces/buttons.h"
#include "rtc.h"
#include "power.h"
#include "screens/scr_init.h"
#include "mode/ui_mode.h"
#include "mode/ui_mode_dispatcher.h"


static const UiDisplay_t display = {
        .clear = display_buffer_clear,
        .put_pixel = display_buffer_put_pixel,
        .text_set_page = display_text_set_page,
        .text_set_offset_x = display_text_set_offset_x,
        .text_set_align = display_text_set_align,
        .text_set_font = display_text_set_font,
        .put_text = display_buffer_put_text,
        .invert = display_buffer_invert,
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

    settings_init();

    uart1_init();

    hmi_btn_init();

    gpio_setup(GPIOA, 0, GPIO_OUT_AF_OD, GPIO_MODE_OUT_50MHZ); // 1-Wire Timer CH1/2 GPIO
    ow_start_bus();
    poll_timer_init(POLL_TIMER);

    tsd_init(&ow_line);

    gpio_reset(GPIOC, 13);

    gpio_setup(GPIOB, 10, GPIO_OUT_AF_OD, GPIO_MODE_OUT_50MHZ); // Display I2C
    gpio_setup(GPIOB, 11, GPIO_OUT_AF_OD, GPIO_MODE_OUT_50MHZ); // Display I2C

    // Init Display I2C (I2C2)
    i2c_setup_timing_sm(DISPLAY_I2C, SystemCoreClock / 2);
    i2c_enable(DISPLAY_I2C);

    display_init();

    display_buffer_clear();
    display_flush();

    // Just to show that device is working,
    // buuut smb is a little bit slow ;)
    // No needed to call event handling for this screen
    SCR_INIT.start();
    SCR_INIT.draw(&display);

    display_flush();

    rtc_init();

    ui_mode_dispr_init(&display);
    ui_mode_dispr_set(UI_MODE_MENU);

    pwr_schedule_sleep();

    while (true) {
        tsd_dispatch_state();

        ui_mode_dispr_dispatch();

        display_flush();

        pwr_sleep_tick();
    }

    return 0;
}
