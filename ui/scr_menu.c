#include "scr_menu.h"

#include <stdbool.h>
#include "../utils.h"
#include "../ssd1306.h"
#include "../display.h"
#include "ui_mode_dispatcher.h"
#include "ui_scr_common.h"
#include "ui_scr_construction.h"
#include "ui_screen_temp.h"

#include "../text_res.h"

#include "bebas_16.h"

#define UI_SCREEN_MENU_HOR_OFFSET 0x10u
#define UI_SCREEN_MENU_VER_OFFSET 0x02u

#define UI_SCREEN_MENU_ITEM_HEIGHT 0x02u

static void __ui_scr_menu_start();
static void __ui_scr_menu_draw(const UiDisplay_t *display);
static void __ui_scr_menu_handle_button(const HmiBtnEvent_t event);


const UiScreen_t SCR_MENU = {
        .start = __ui_scr_menu_start,
        .draw = __ui_scr_menu_draw,
        .handle_button = __ui_scr_menu_handle_button,
};

typedef struct MenuItem {
    const uint8_t *title;
    const UiMode_t mode;
} MenuItem_t;

static const MenuItem_t MENU[] = {
        {
            .title = RES_STR_SCR_MENU_TEMPERATURE,
            .mode = UI_MODE_TEMPERATURE,
        },
        {
            .title = RES_STR_SCR_MENU_TIME,
            .mode = UI_MODE_TIME,
        },
        {
            .title = RES_STR_SCR_MENU_SETTINGS,
            .mode = UI_MODE_SETTINGS,
        },
        {
            .title = RES_STR_SCR_MENU_ABOUT,
            .mode = UI_MODE_INFO,
        },
};

static uint32_t s_menu_cursor_pos = 0u;
static uint32_t s_menu_item_first = 0u;
static UiMode_t s_mode_current = UI_MODE_TIME;

static FORCE_INLINE uint32_t __ui_scr_menu_item_count() {
    return LENGTH_OF(MENU);
}

static FORCE_INLINE uint32_t __scr_menu_items_per_menu_page() {
    uint32_t rows_display = DISPLAY_HEIGHT / SSD1306_PIXEL_PER_PAGE;
    uint32_t rows_menu = rows_display - UI_SCREEN_MENU_VER_OFFSET;
    return rows_menu / UI_SCREEN_MENU_ITEM_HEIGHT;
}

static FORCE_INLINE uint32_t __ui_scr_menu_draw_count() {
    uint32_t total_count = __ui_scr_menu_item_count();
    uint32_t disp_menu_rows = (DISPLAY_HEIGHT / SSD1306_PIXEL_PER_PAGE) - UI_SCREEN_MENU_VER_OFFSET;
    uint32_t per_page_max = disp_menu_rows / UI_SCREEN_MENU_ITEM_HEIGHT;
    return (total_count < per_page_max) ? total_count : per_page_max;
}

static void __ui_scr_menu_sel_next() {
    if (UI_MODE_NONE == s_mode_current) {
        return;
    }

    s_mode_current = (s_mode_current + 1u) % UI_MODE_COUNT;

    uint32_t menu_item_count = __ui_scr_menu_item_count();
    if (menu_item_count <= 0) {
        return;
    }

    uint32_t draw_count = __ui_scr_menu_draw_count();

    if (menu_item_count < draw_count && s_menu_cursor_pos < (menu_item_count - 1)) {
        s_menu_cursor_pos++;
        return;
    }

    uint32_t menu_first_max_idx = menu_item_count - draw_count;

    if (s_menu_cursor_pos < (draw_count - 1)) {
        s_menu_cursor_pos++;
    } else if (s_menu_item_first < menu_first_max_idx) {
        s_menu_item_first++;
    } else {
        s_menu_cursor_pos = 0u;
        s_menu_item_first = 0u;
    }
}

static void __ui_scr_menu_sel_prev() {
    if (UI_MODE_NONE == s_mode_current) {
        return;
    }

    if (s_mode_current == 0x0u) {
        s_mode_current = (UI_MODE_COUNT - 1u);
    }

    if (s_menu_cursor_pos > 0) {
        s_menu_cursor_pos--;
    } else if (s_menu_item_first > 0) {
        s_menu_item_first--;
    } else {
        uint32_t draw_count = __ui_scr_menu_draw_count();
        s_menu_cursor_pos = draw_count - 1;
        s_menu_item_first = __ui_scr_menu_item_count() - draw_count;
    }
}

static void __ui_scr_menu_cursor_init(uint32_t start_index) {
    uint32_t count = __ui_scr_menu_item_count();
    if (count == 0 || start_index >= UI_MODE_COUNT) {
        return;
    }

    s_mode_current = start_index;

    // The most first menu item
    if (0u == start_index) {
        s_menu_cursor_pos = 0u;
        s_menu_item_first = 0u;
        return;
    }

    uint32_t draw_count = __ui_scr_menu_draw_count();

    // The last one
    if ((count - 1) == start_index) {
        s_menu_cursor_pos = draw_count - 1;
        s_menu_item_first = count - draw_count;
        return;
    }

    s_menu_cursor_pos = (draw_count - 1) / 2;
    s_menu_item_first = start_index - s_menu_cursor_pos;
}

static void __ui_scr_menu_start() {
    __ui_scr_menu_cursor_init(ui_mode_dispr_get());
}

static void __ui_scr_menu_draw(const UiDisplay_t *display) {
    display->clear();

    display->text_set_font(ssd1306_bebas_16_get_glyph);
    ui_scr_draw_title(display, RES_STR_SCR_MENU_TITLE);

    display->text_set_align(TEXT_ALIGN_LEFT);
    display->text_set_font(ssd1306_bebas_16_get_glyph);

    display->text_set_offset_x(UI_SCREEN_MENU_HOR_OFFSET);

    uint32_t draw_count = __ui_scr_menu_draw_count();

    for (uint32_t idx = 0u; idx < draw_count; idx++) {
        uint32_t menu_idx = s_menu_item_first + idx;
        uint32_t disp_row_idx = UI_SCREEN_MENU_VER_OFFSET + idx * UI_SCREEN_MENU_ITEM_HEIGHT;

        display->text_set_page((uint8_t) disp_row_idx);
        display->put_text(MENU[menu_idx].title);
    }

    uint8_t cur_top = s_menu_cursor_pos * UI_SCREEN_MENU_ITEM_HEIGHT + UI_SCREEN_MENU_VER_OFFSET;
    for (uint32_t idx = 0; idx < UI_SCREEN_MENU_ITEM_HEIGHT; idx++) {
        display->invert(cur_top + idx, 0x0u, DISPLAY_WIDTH - 0x01u);
    }
}

static void __ui_scr_menu_handle_button(const HmiBtnEvent_t event) {
    if (event.btn == HMI_BTN_RIGHT && event.type == HMI_BTN_EVENT_PRESS) {
        __ui_scr_menu_sel_next();
    } else if (event.btn == HMI_BTN_LEFT && event.type == HMI_BTN_EVENT_PRESS) {
        __ui_scr_menu_sel_prev();
    } else if (event.btn == HMI_BTN_ENTER && event.type == HMI_BTN_EVENT_PRESS) {
        ui_mode_dispr_set(s_mode_current);
    }
}
