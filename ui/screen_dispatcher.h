#ifndef THERMOMETER_UI_SCREEN_CURRENT_H
#define THERMOMETER_UI_SCREEN_CURRENT_H

#include "ui_screen.h"

void scr_dispatcher_init(const UiDisplay_t *display);

const UiScreen_t *scr_dispatcher_get_current();

void scr_dispatcher_set_current(const UiScreen_t *scr);

void scr_dispatcher_dispatch();

#endif //THERMOMETER_UI_SCREEN_CURRENT_H
