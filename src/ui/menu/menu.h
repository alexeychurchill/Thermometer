#ifndef THERMOMETER_MENU_H
#define THERMOMETER_MENU_H
#include <stdint.h>
#include "../screens/ui_screen.h"

typedef struct Menu {
    uint32_t first_item_index;
    uint32_t cursor_position;
    uint32_t item_count;
    uint32_t last_picked_item_index;
    const uint8_t** titles;
    uint32_t top_margin_pages;
    uint32_t left_margin_px;
    uint32_t item_height_pages;
} Menu_t;

void menu_init_cursor(Menu_t *menu, uint32_t start_index);

void menu_select_next(Menu_t *menu);

void menu_select_previous(Menu_t *menu);

uint32_t menu_pick_item(Menu_t *menu);

void menu_render(const Menu_t *menu, const UiDisplay_t *display);

#endif //THERMOMETER_MENU_H
