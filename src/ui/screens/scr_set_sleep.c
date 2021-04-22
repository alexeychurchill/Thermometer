#include "scr_set_sleep.h"
#include "scr_common.h"
#include "../menu/menu.h"
#include "ui_mode_dispatcher.h"
#include "../../text_res.h"
#include "../../flash_settings.h"

#define ITEM_COUNT                                      8u
#define ITEM_OPTIONS_START                              1u
#define ITEM_FIRST_DEFAULT                              0u

static const uint32_t SLEEP_PERIOD_MS[] = {
              0u, // Off
          60000u, // 1m * 60s * 1000ms
         300000u, // 5m * 60s * 1000ms
         900000u, // 15m * 60s * 1000ms
        1800000u, // 30m * 60s * 1000ms
        3600000u, // 1h * 60m * 60s * 1000ms
        7200000u, // 2h * 60m * 60s * 1000ms
};

static const uint8_t * const SLEEP_PERIOD_TITLES[]  = {
        RES_STR_GENERIC_BACK,
        RES_STR_SCR_SET_SLEEP_ITEM_OFF,
        RES_STR_SCR_SET_SLEEP_ITEM_MIN_1,
        RES_STR_SCR_SET_SLEEP_ITEM_MIN_5,
        RES_STR_SCR_SET_SLEEP_ITEM_MIN_15,
        RES_STR_SCR_SET_SLEEP_ITEM_MIN_30,
        RES_STR_SCR_SET_SLEEP_ITEM_HR_1,
        RES_STR_SCR_SET_SLEEP_ITEM_HR_2,
};

static Menu_t SET_SLEEP_MENU = {
        .first_item_index = 0u,
        .cursor_position = 0u,
        .item_count = ITEM_COUNT,
        .last_picked_item_index = ITEM_FIRST_DEFAULT,
        .titles = (const uint8_t**) SLEEP_PERIOD_TITLES,
        .top_margin_pages = MENU_DEFAULT_TOP_MARGIN_PAGES,
        .left_margin_px = MENU_DEFAULT_LEFT_MARGIN_PX,
        .item_height_pages = MENU_DEFAULT_ITEM_HEIGHT_PAGES,
};

static void start();
static void draw(const UiDisplay_t *display);
static void handle_button(HmiBtnEvent_t event);

const UiScreen_t SCR_SET_SLEEP = {
        .start = start,
        .draw = draw,
        .handle_button = handle_button,
};

static void start() {
    uint32_t sleep_delay = settings_get_sleep();

    for (uint32_t index = 0u; index < ITEM_COUNT - ITEM_OPTIONS_START; index++) {
        if (SLEEP_PERIOD_MS[index] == sleep_delay) {
            menu_init_cursor(&SET_SLEEP_MENU, index + ITEM_OPTIONS_START);
            return;
        }
    }
}

static void draw(const UiDisplay_t *display) {
    display->clear();
    menu_render(&SET_SLEEP_MENU, display);
}

static void pick_item(uint32_t picked_index) {
    uint32_t sleep_delay = SLEEP_PERIOD_MS[picked_index - ITEM_OPTIONS_START];
    settings_set_sleep(sleep_delay);
    ui_mode_dispr_set(UI_MODE_SETTINGS);
}

static void handle_button(const HmiBtnEvent_t event) {
    if (event.btn == HMI_BTN_ENTER && event.type == HMI_BTN_EVENT_LONG_PRESS) {
        ui_mode_dispr_set(UI_MODE_SETTINGS);
    } else if (event.btn == HMI_BTN_ENTER && event.type == HMI_BTN_EVENT_PRESS) {
        uint32_t picked = menu_pick_item(&SET_SLEEP_MENU);
        pick_item(picked);
    } else if (event.btn == HMI_BTN_LEFT && event.type == HMI_BTN_EVENT_PRESS) {
        menu_select_previous(&SET_SLEEP_MENU);
    } else if (event.btn == HMI_BTN_RIGHT && event.type == HMI_BTN_EVENT_PRESS) {
        menu_select_next(&SET_SLEEP_MENU);
    }
}
