#include "scr_construction.h"
#include "ui_screen.h"
#include "interfaces/buttons.h"
#include "ui_mode_dispatcher.h"

#include "text_res.h"

#include "bebas_16.h"

#define SCR_CONSTRUCTION_MSG_PAGE_LINE_0        2u
#define SCR_CONSTRUCTION_MSG_PAGE_LINE_1        (SCR_CONSTRUCTION_MSG_PAGE_LINE_0 + 2u)

/**
 * This screen implementation can be used as a reference one
 *
 * Screen implementation function naming rules:
 * static [return type] __scr_[screen name]_[callback name](..)
 *
 * Structure of screen implementation source file:
 * 1. Declarations of screen callback functions
 * 2. [Optional] Declarations of private functions, which,
 * probably, needed in functions defined before
 * 3. Definition of the const structure with screen callbacks
 * 4. Definitions of screen callback functions
 * 5. [Optional] Private screen implementation functions
 */

static void __scr_construction_start();
static void __scr_construction_draw(const UiDisplay_t*);
static void __scr_construction_handle_buttons(const HmiBtnEvent_t);

const UiScreen_t SCR_CONSTRUCTION = {
        .start = __scr_construction_start,
        .draw = __scr_construction_draw,
        .handle_button = __scr_construction_handle_buttons,
};

static void __scr_construction_start() {
    // No op
}

static void __scr_construction_draw(const UiDisplay_t *display) {
    display->clear();

    display->text_set_align(TEXT_ALIGN_CENTER);
    display->text_set_font(ssd1306_bebas_16_get_glyph);

    display->text_set_page(SCR_CONSTRUCTION_MSG_PAGE_LINE_0);
    display->put_text(RES_STR_SCR_CONSTRUCTION_MSG_LINE_0);

    display->text_set_page(SCR_CONSTRUCTION_MSG_PAGE_LINE_1);
    display->put_text(RES_STR_SCR_CONSTRUCTION_MSG_LINE_1);
}

static void __scr_construction_handle_buttons(const HmiBtnEvent_t event) {
    if (event.btn == HMI_BTN_ENTER) {
        ui_mode_dispr_set(UI_MODE_MENU);
    }
}
