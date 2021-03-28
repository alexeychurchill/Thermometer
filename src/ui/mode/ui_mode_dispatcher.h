#ifndef THERMOMETER_UI_MODE_DISPATCHER_H
#define THERMOMETER_UI_MODE_DISPATCHER_H

#include "ui_screen.h"
#include "mode/ui_mode.h"

void ui_mode_dispr_init(const UiDisplay_t *display);

void ui_mode_dispr_set(UiMode_t mode);

UiMode_t ui_mode_dispr_get();

void ui_mode_dispr_dispatch();

#endif //THERMOMETER_UI_MODE_DISPATCHER_H
