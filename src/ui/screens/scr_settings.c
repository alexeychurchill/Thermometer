#include "scr_settings.h"
#include "ui_screen.h"
#include "scr_common.h"
#include "../mode/ui_mode_dispatcher.h"
#include "../menu/menu.h"
#include "../../text_res.h"

#define ITEM_COUNT                              3u
#define ITEM_FIRST_DEFAULT                      0u
#define ITEM_GO_BACK_INDEX                      0u
#define ITEM_GO_BACK_OFFSET                     1u

static void start();
static void draw(const UiDisplay_t *display);
static void handle_button(HmiBtnEvent_t event);

const UiScreen_t SCR_SETTINGS = {
        .start = start,
        .draw = draw,
        .handle_button = handle_button,
};

static const uint8_t *ITEM_TITLES[] = {
        RES_STR_GENERIC_BACK,
        RES_STR_SCR_SETTINGS_ITEM_TIME,
        RES_STR_SCR_SETTINGS_ITEM_SLEEP,
};

static const UiMode_t ITEM_MODES[] = {
        UI_MODE_SET_TIME,
        UI_MODE_SET_SLEEP,
};

static Menu_t SETTINGS_MENU = {
        .first_item_index = 0u,
        .cursor_position = 0u,
        .item_count = ITEM_COUNT,
        .last_picked_item_index = ITEM_FIRST_DEFAULT,
        .titles = (const uint8_t**) ITEM_TITLES,
        .top_margin_pages = MENU_DEFAULT_TOP_MARGIN_PAGES,
        .left_margin_px = MENU_DEFAULT_LEFT_MARGIN_PX,
        .item_height_pages = MENU_DEFAULT_ITEM_HEIGHT_PAGES,
};

static void start() {
    menu_init_cursor(&SETTINGS_MENU, ITEM_FIRST_DEFAULT);
}

static void draw(const UiDisplay_t *display) {
    display->clear();

    scr_common_draw_title(display, RES_STR_SCR_SETTINGS_TITLE);
    menu_render(&SETTINGS_MENU, display);
}

static void pick_settings_item() {
    uint32_t item_index = menu_pick_item(&SETTINGS_MENU);
    if (item_index == ITEM_GO_BACK_INDEX) {
        ui_mode_dispr_set(UI_MODE_MENU);
        return;
    }

    UiMode_t ui_mode = ITEM_MODES[item_index - ITEM_GO_BACK_OFFSET];
    ui_mode_dispr_set(ui_mode);
}

static void handle_button(const HmiBtnEvent_t event) {
    if (event.btn == HMI_BTN_ENTER && event.type == HMI_BTN_EVENT_LONG_PRESS) {
        ui_mode_dispr_set(UI_MODE_MENU);
    } else if (event.btn == HMI_BTN_ENTER && event.type == HMI_BTN_EVENT_PRESS) {
        pick_settings_item();
    } else if (event.btn == HMI_BTN_RIGHT && event.type == HMI_BTN_EVENT_PRESS) {
        menu_select_next(&SETTINGS_MENU);
    } else if (event.btn == HMI_BTN_LEFT && event.type == HMI_BTN_EVENT_PRESS) {
        menu_select_previous(&SETTINGS_MENU);
    }
}
