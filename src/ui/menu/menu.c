#include "menu.h"
#include "config.h"
#include "../drivers/ssd1306.h"

static uint32_t get_visible_count(const Menu_t *menu) {
    uint32_t display_rows = (DISPLAY_HEIGHT / SSD1306_PIXEL_PER_PAGE) - menu->top_margin_pages;
    uint32_t items_per_page = display_rows / menu->item_height_pages;
    return (menu->item_count < items_per_page) ? menu->item_count : items_per_page;
}

void menu_init_cursor(Menu_t *menu, uint32_t start_index) {
    uint32_t item_count = menu->item_count;
    if (item_count == 0u || start_index >= item_count) {
        return;
    }

    // The most first menu item
    if (0u == start_index) {
        menu->cursor_position = 0u;
        menu->first_item_index = 0u;
        return;
    }

    uint32_t visible_count = get_visible_count(menu);

    // The last one
    if (start_index == (item_count - 1u)) {
        menu->cursor_position = visible_count - 1u;
        menu->first_item_index = item_count - visible_count;
        return;
    }

    menu->cursor_position = (visible_count - 1) / 2;
    menu->first_item_index = start_index - menu->cursor_position;
}

void menu_select_next(Menu_t *menu) {
    if (menu->item_count <= 0u) {
        return;
    }

    uint32_t visible_count = get_visible_count(menu);
    if (menu->item_count < visible_count && menu->cursor_position < (menu->item_count - 1u)) {
        menu->cursor_position++;
        return;
    }

    uint32_t first_max_index = menu->item_count - visible_count;
    if (menu->cursor_position < (visible_count - 1u)) {
        menu->cursor_position++;
    } else if (menu->first_item_index < first_max_index) {
        menu->first_item_index++;
    } else {
        menu->cursor_position = 0u;
        menu->first_item_index = 0u;
    }
}

void menu_select_previous(Menu_t *menu) {
    if (menu->item_count <= 0u) {
        return;
    }

    if (menu->cursor_position > 0u) {
        menu->cursor_position--;
    } else if (menu->first_item_index > 0u) {
        menu->first_item_index--;
    } else {
        uint32_t visible_count = get_visible_count(menu);
        menu->cursor_position = visible_count - 1u;
        menu->first_item_index = menu->item_count - visible_count;
    }
}

uint32_t menu_pick_item(Menu_t *menu) {
    uint32_t picked_index = menu->first_item_index + menu->cursor_position;
    menu->last_picked_item_index = picked_index;
    return picked_index;
}

void menu_render(const Menu_t *menu, const UiDisplay_t *display) {
    display->text_set_offset_x(menu->left_margin_px);

    uint32_t visible_count = get_visible_count(menu);

    for (uint32_t visible_item_n = 0u; visible_item_n < visible_count; visible_item_n++) {
        uint32_t menu_item_n = menu->first_item_index + visible_item_n;
        uint32_t disp_page_n = menu->top_margin_pages + visible_item_n * menu->item_height_pages;

        display->text_set_page((uint8_t) disp_page_n);
        display->put_text(menu->titles[menu_item_n]);
    }

    uint8_t cursor_top = menu->cursor_position * menu->item_height_pages + menu->top_margin_pages;
    for (uint32_t displ_page_n = 0; displ_page_n < menu->item_height_pages; displ_page_n++) {
        display->invert(cursor_top + displ_page_n, 0x0u, DISPLAY_WIDTH - 0x01u);
    }
}
