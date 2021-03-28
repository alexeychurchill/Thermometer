#include "scr_menu.h"
#include "util/utils.h"
#include "ui_mode_dispatcher.h"
#include "scr_common.h"
#include "../menu/menu.h"
#include "text_res.h"
#include "bebas_16.h"

#define SCR_MENU_LAST_PICKED_DEFAULT                0x0u
#define SCR_MENU_ITEM_COUNT                         0x4u

static void start();
static void draw(const UiDisplay_t *display);
static void handle_button(HmiBtnEvent_t event);

const UiScreen_t SCR_MENU = {
        .start = start,
        .draw = draw,
        .handle_button = handle_button,
};

static const uint8_t* MENU_TITLES[] = {
        RES_STR_SCR_MENU_TEMPERATURE,
        RES_STR_SCR_MENU_TIME,
        RES_STR_SCR_MENU_SETTINGS,
        RES_STR_SCR_MENU_ABOUT,
};

static const UiMode_t MENU_MODES[] = {
        UI_MODE_TEMPERATURE,
        UI_MODE_TIME,
        UI_MODE_SETTINGS,
        UI_MODE_INFO,
};

static Menu_t menu = {
        .first_item_index = 0u,
        .cursor_position = 0u,
        .item_count = SCR_MENU_ITEM_COUNT,
        .last_picked_item_index = SCR_MENU_LAST_PICKED_DEFAULT,
        .titles = (const uint8_t**) MENU_TITLES,
        .top_margin_pages = MENU_DEFAULT_TOP_MARGIN_PAGES,
        .left_margin_px = MENU_DEFAULT_LEFT_MARGIN_PX,
        .item_height_pages = MENU_DEFAULT_ITEM_HEIGHT_PAGES,
};

static void start() {
    menu_init_cursor(&menu, menu.last_picked_item_index);
}

static void draw(const UiDisplay_t *display) {
    display->clear();

    display->text_set_font(ssd1306_bebas_16_get_glyph);
    scr_common_draw_title(display, RES_STR_SCR_MENU_TITLE);

    display->text_set_align(TEXT_ALIGN_LEFT);
    display->text_set_font(ssd1306_bebas_16_get_glyph);

    menu_render(&menu, display);
}

static void pick_menu_item() {
    uint32_t picked_index = menu_pick_item(&menu);
    UiMode_t mode = MENU_MODES[picked_index];
    ui_mode_dispr_set(mode);
}

static void handle_button(const HmiBtnEvent_t event) {
    if (event.btn == HMI_BTN_RIGHT && event.type == HMI_BTN_EVENT_PRESS) {
        menu_select_next(&menu);
    } else if (event.btn == HMI_BTN_LEFT && event.type == HMI_BTN_EVENT_PRESS) {
        menu_select_previous(&menu);
    } else if (event.btn == HMI_BTN_ENTER && event.type == HMI_BTN_EVENT_PRESS) {
        pick_menu_item();
    }
}
