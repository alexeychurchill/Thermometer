#include "screen_dispatcher.h"

#include <stddef.h>

#include "../buttons.h"

static UiDisplay_t *s_display = NULL;
static UiScreen_t *s_scr_current = NULL;

void scr_dispatcher_init(const UiDisplay_t *display) {
    s_display = display;
}

const UiScreen_t* scr_dispatcher_get_current() {
    return s_scr_current;
}

void scr_dispatcher_set_current(const UiScreen_t *scr) {
    if (scr != NULL && scr->start != NULL) {
        // TODO: Uncomment after implementation for all the screens
        /*scr->start();*/
    }
    s_scr_current = scr;
}

void scr_dispatcher_dispatch() {
    if (s_scr_current == NULL || s_display == NULL) {
        return;
    }

    if (hmi_btn_has_event()) {
        HmiBtnEvent_t btn_event = hmi_btn_poll_event();
        s_scr_current->handle_button(btn_event);
    }

    s_scr_current->draw(s_display);
}