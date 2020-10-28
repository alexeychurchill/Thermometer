#ifndef THERMOMETER_SSD1306_H
#define THERMOMETER_SSD1306_H

#include <stdint.h>
#include <stdbool.h>


typedef void (*SSD1306SendFunc_t)(uint32_t, const uint8_t[]);

typedef enum SSD1306Cmd {
    // Fundamental commands
    SSD1306_CMD_SET_CONTRAST                = 0x81,
    SSD1306_CMD_ENTIRE_DISPLAY_OFF          = 0xA4,
    SSD1306_CMD_ENTIRE_DISPLAY_ON           = 0xA5,
    SSD1306_CMD_INVERSE_OFF                 = 0xA6,
    SSD1306_CMD_INVERSE_ON                  = 0xA7,
    SSD1306_CMD_DISPLAY_OFF                 = 0xAE,
    SSD1306_CMD_DISPLAY_ON                  = 0xAF,

    // Scrolling commands
    SSD1306_CMD_SCROLL_SETUP_H_LEFT         = 0x27,
    SSD1306_CMD_SCROLL_SETUP_H_RIGHT        = 0x26,
    SSD1306_CMD_SCROLL_SETUP_VH_LEFT        = 0x2A,
    SSD1306_CMD_SCROLL_SETUP_VH_RIGHT       = 0x29,
    SSD1306_CMD_SCROLL_DEACTIVATE           = 0x2E,
    SSD1306_CMD_SCROLL_ACTIVATE             = 0x2F,
    SSD1306_CMD_SCROLL_SETUP_V_AREA         = 0xA3,

    // Addressing commands
    SSD1306_CMD_ADDR_START_COL_LOWER        = 0x00, // OR with arg (4 bit)
    SSD1306_CMD_ADDR_START_COL_HIGHER       = 0x10, // OR with arg (4 bit)
    SSD1306_CMD_ADDR_SET_ADDR_MODE          = 0x20,
    SSD1306_CMD_ADDR_SET_COLUMN             = 0x21,
    SSD1306_CMD_ADDR_SET_PAGE               = 0x22,
    SSD1306_CMD_ADDR_START_PAGE             = 0xB0, // OR with arg (3 bit)

    // HW configuration commands
    SSD1306_CMD_HW_SET_START_LINE           = 0x40, // OR with arg (6 bit)
    SSD1306_CMD_HW_SEGMENT_REMAP_ON         = 0xA0,
    SSD1306_CMD_HW_SEGMENT_REMAP_OFF        = 0xA1,
    SSD1306_CMD_HW_SET_MULTIPLEX_RATIO      = 0xA8,
    SSD1306_CMD_HW_SET_COM_SCAN_DIR_NORMAL  = 0xC0,
    SSD1306_CMD_HW_SET_COM_SCAN_DIR_REMAP   = 0xC8,
    SSD1306_CMD_HW_SET_DISPLAY_OFFSET       = 0xD3,
    SSD1306_CMD_HW_SET_COM_PINS_CONFIG      = 0xDA,

    // Timings & Driving scheme configuration commands
    SSD1306_CMD_SET_DIV_RATIO_OSC_FREQ      = 0xD5,
    SSD1306_CMD_SET_PRE_CHARGE_PERIOD       = 0xD9,
    SSD1306_CMD_SET_VCOMH_DESELECT_LEVEL    = 0xDB,
    SSD1306_CMD_NOP                         = 0xE3,
} SSD1306Cmd_t;

typedef enum SSD1306AddrMode {
    SSD1306_ADDR_MODE_HORIZONTAL            = 0x00,
    SSD1306_ADDR_MODE_VERTICAL              = 0x01,
    SSD1306_ADDR_MODE_PAGE                  = 0x02,
} SSD1306AddrMode_t;

typedef enum SSD1306CtrlByte {
    SSD1306_CTRL_COMMAND                    = 0x00,
    SSD1306_CTRL_DATA                       = 0x40,
    SSD1306_CTRL_CO_COMMAND                 = 0x80,
    SSD1306_CTRL_CO_DATA                    = 0xC0 // 0x80 | 0x40
} SSD1306CtrlByte_t;

typedef enum SSD1306DeselectLevel {
    SSD1306_DESELECT_LEVEL_0_65VCC          = 0x00,
    SSD1306_DESELECT_LEVEL_0_77VCC          = 0x20, // Reset value
    SSD1306_DESELECT_LEVEL_0_83VCC          = 0x30,
} SSD1306DeselectLevel_t;


#endif //THERMOMETER_SSD1306_H
