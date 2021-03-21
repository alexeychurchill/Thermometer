#include "ui_mode_dispatcher.h"

#include <stddef.h>

#include "../buttons.h"

#include "ui_screen_temp.h"
#include "scr_menu.h"
#include "scr_time.h"
#include "scr_set_time.h"
#include "scr_construction.h"

static UiMode_t s_mode = UI_MODE_NONE;
static UiDisplay_t *s_display = NULL;
static UiScreen_t *s_scr_current = NULL;

static const UiScreen_t* const MODES_SCREENS[UI_MODE_COUNT] = {
        [UI_MODE_MENU] = &SCR_MENU,
        [UI_MODE_TEMPERATURE] = &UI_SCREEN_TEMP,
        [UI_MODE_TIME] = &SCR_TIME,
        [UI_MODE_SETTINGS] = &SCR_CONSTRUCTION,
        [UI_MODE_INFO] = &SCR_CONSTRUCTION,
        [UI_MODE_SET_TIME] = &SCR_SET_TIME
};

void ui_mode_dispr_init(const UiDisplay_t *display) {
    s_display = display;
}

void ui_mode_dispr_set(const UiMode_t mode) {
    if (UI_MODE_COUNT <= mode) {
        return;
    }

    s_mode = mode;

    const UiScreen_t *scr = MODES_SCREENS[mode];
    if (scr->start != NULL) {
        scr->start();
    }
}

const UiMode_t ui_mode_dispr_get() {
    return s_mode;
}

void ui_mode_dispr_dispatch() {
    if (s_display == NULL) {
        return;
    }

    const UiScreen_t *scr = MODES_SCREENS[s_mode];

    if (hmi_btn_has_event()) {
        HmiBtnEvent_t btn_event = hmi_btn_poll_event();
        scr->handle_button(btn_event);
    }

    scr->draw(s_display);
}
