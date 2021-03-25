#ifndef THERMOMETER_SCR_INIT_H
#define THERMOMETER_SCR_INIT_H

#include "ui_screen.h"

/**
 * TODO: Get rid of this screen!
 *
 * Actually, we need to show this to wait while some peripherals
 * take their time to init. Consider replacing this with some kinda
 * of polling, or, even, using some corresponding IRQs.
 *
 * Hint: rtc.c, rtc_init(), __rtc_start(), __rtc_wait_resync()
 */
const UiScreen_t SCR_INIT;

#endif //THERMOMETER_SCR_INIT_H
