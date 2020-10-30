#ifndef THERMOMETER_SSD1306_H
#define THERMOMETER_SSD1306_H

#include <stdint.h>
#include <stdbool.h>


#define SSD1306_CMD_DUMMY_BYTE_00            0x00u
#define SSD1306_CMD_DUMMY_BYTE_FF            0xFFu
#define SSD1306_PAGE_MASK                    0x07u
#define SSD1306_COL_MASK                     0x7Fu
#define SSD1306_ROW_MASK                     0x3Fu
#define SSD1306_SCROLL_INTERVAL_MASK         0x07u


#define SSD1306_DEF_CMD(name, cmd)                                                  \
    inline __attribute__((always_inline)) void                                      \
    ssd1306_##name(SSD1306SendFunc_t send_func) {                                   \
        const uint8_t bytes[] = { SSD1306_CTRL_COMMAND, (uint8_t) (cmd) };          \
        send_func(bytes, sizeof(bytes));                                            \
    }

#define SSD1306_DEF_CMD_WITH_ARG(name, cmd, arg_type, arg_name)                     \
    inline __attribute__((always_inline)) void                                      \
    ssd1306_##name(SSD1306SendFunc_t send_func, arg_type arg_name) {                \
        const uint8_t bytes[] = {                                                   \
            SSD1306_CTRL_COMMAND,                                                   \
            (uint8_t) (cmd),                                                        \
            (uint8_t) (arg_name)                                                    \
        };                                                                          \
        send_func(bytes, sizeof(bytes));                                            \
    }

#define SSD1306_MULTIPLE(args...)            args

#define SSD1306_DEF_CMD_WITH_ARGS(name, args, data)                                 \
    inline __attribute__((always_inline)) void                                      \
    ssd1306_##name(SSD1306SendFunc_t send_bytes, args) {                            \
        const uint8_t bytes[] = {                                                   \
            data                                                                    \
        };                                                                          \
        send_bytes(bytes, sizeof(bytes));                                           \
    }

#define SSD1306_COL_LOWER(col)              ((uint8_t) ((col) & 0x0Fu))

#define SSD1306_COL_HIGHER(col)             ((uint8_t) (((col) & 0xF0u) >> 0x4u))


typedef void (*SSD1306SendFunc_t)(const uint8_t*, uint32_t);


typedef enum SSD1306Cmd {
    // Fundamental commands
    SSD1306_CMD_SET_CONTRAST                = 0x81u,
    SSD1306_CMD_ENTIRE_DISPLAY_OFF          = 0xA4u,
    SSD1306_CMD_ENTIRE_DISPLAY_ON           = 0xA5u,
    SSD1306_CMD_INVERSE_OFF                 = 0xA6u,
    SSD1306_CMD_INVERSE_ON                  = 0xA7u,
    SSD1306_CMD_DISPLAY_OFF                 = 0xAEu,
    SSD1306_CMD_DISPLAY_ON                  = 0xAFu,

    // Scrolling commands
    SSD1306_CMD_SCROLL_SETUP_H_LEFT         = 0x27u,
    SSD1306_CMD_SCROLL_SETUP_H_RIGHT        = 0x26u,
    SSD1306_CMD_SCROLL_SETUP_VH_LEFT        = 0x2Au,
    SSD1306_CMD_SCROLL_SETUP_VH_RIGHT       = 0x29u,
    SSD1306_CMD_SCROLL_DEACTIVATE           = 0x2Eu,
    SSD1306_CMD_SCROLL_ACTIVATE             = 0x2Fu,
    SSD1306_CMD_SCROLL_SETUP_V_AREA         = 0xA3u,

    // Addressing commands
    SSD1306_CMD_ADDR_START_COL_LOWER        = 0x00u, // OR with arg (4 bit)
    SSD1306_CMD_ADDR_START_COL_HIGHER       = 0x10u, // OR with arg (4 bit)
    SSD1306_CMD_ADDR_SET_ADDR_MODE          = 0x20u,
    SSD1306_CMD_ADDR_SET_COLUMN             = 0x21u,
    SSD1306_CMD_ADDR_SET_PAGE               = 0x22u,
    SSD1306_CMD_ADDR_START_PAGE             = 0xB0u, // OR with arg (3 bit)

    // HW configuration commands
    SSD1306_CMD_HW_SET_START_LINE           = 0x40u, // OR with arg (6 bit)
    SSD1306_CMD_HW_SEGMENT_REMAP_ON         = 0xA0u,
    SSD1306_CMD_HW_SEGMENT_REMAP_OFF        = 0xA1u,
    SSD1306_CMD_HW_SET_MULTIPLEX_RATIO      = 0xA8u,
    SSD1306_CMD_HW_SET_COM_SCAN_DIR_NORMAL  = 0xC0u,
    SSD1306_CMD_HW_SET_COM_SCAN_DIR_REMAP   = 0xC8u,
    SSD1306_CMD_HW_SET_DISPLAY_OFFSET       = 0xD3u,
    SSD1306_CMD_HW_SET_COM_PINS_CONFIG      = 0xDAu,

    // Timings & Driving scheme configuration commands
    SSD1306_CMD_SET_DIV_RATIO_OSC_FREQ      = 0xD5u,
    SSD1306_CMD_SET_PRE_CHARGE_PERIOD       = 0xD9u,
    SSD1306_CMD_SET_VCOMH_DESELECT_LEVEL    = 0xDBu,
    SSD1306_CMD_NOP                         = 0xE3u,

    // Charge pump configuration commands
    SSD1306_CMD_CHARGE_PUMP                 = 0x8Du,
} SSD1306Cmd_t;

typedef enum SSD1306CtrlByte {
    SSD1306_CTRL_COMMAND                    = 0x00u,
    SSD1306_CTRL_DATA                       = 0x40u,
    SSD1306_CTRL_CO_COMMAND                 = 0x80u,
    SSD1306_CTRL_CO_DATA                    = 0xC0u // 0x80 | 0x40
} SSD1306CtrlByte_t;

typedef enum SSD1306AddrMode {
    SSD1306_ADDR_MODE_HORIZONTAL            = 0x00u,
    SSD1306_ADDR_MODE_VERTICAL              = 0x01u,
    SSD1306_ADDR_MODE_PAGE                  = 0x02u,
} SSD1306AddrMode_t;

typedef enum SSD1306COMPinsConfig {
    SSD1306_COM_PINS_CONFIG_DEFAULT         = 0x02u,
    SSD1306_COM_PINS_CONFIG_ALTERNATIVE     = 0x12u,
    SSD1306_COM_PINS_CONFIG_LR_REMAP        = 0x22u,
} SSD1306COMPinsConfig_t;

typedef enum SSD1306DeselectLevel {
    SSD1306_DESELECT_LEVEL_0_65VCC          = 0x00u,
    SSD1306_DESELECT_LEVEL_0_77VCC          = 0x20u, // Reset value
    SSD1306_DESELECT_LEVEL_0_83VCC          = 0x30u,
} SSD1306DeselectLevel_t;

typedef enum SSD1306ChargePump {
    SSD1306_CHARGE_PUMP_DISABLE             = 0x10u,
    SSD1306_CHARGE_PUMP_ENABLE              = 0x14u,
} SSD1306ChargePump_t;

// Commands functions

// Fundamental commands

SSD1306_DEF_CMD_WITH_ARG(set_contrast, SSD1306_CMD_SET_CONTRAST, uint8_t, contrast);
SSD1306_DEF_CMD(entire_display_on, SSD1306_CMD_ENTIRE_DISPLAY_ON);
SSD1306_DEF_CMD(entire_display_off, SSD1306_CMD_ENTIRE_DISPLAY_OFF);
SSD1306_DEF_CMD(set_inverse_off, SSD1306_CMD_INVERSE_OFF);
SSD1306_DEF_CMD(set_inverse_on, SSD1306_CMD_INVERSE_ON);
SSD1306_DEF_CMD(set_display_off, SSD1306_CMD_DISPLAY_OFF);
SSD1306_DEF_CMD(set_display_on, SSD1306_CMD_DISPLAY_ON);

// Scrolling commands

#define SSD1306_DEF_CMD_SCROLL_HOR(name, cmd)                                       \
    SSD1306_DEF_CMD_WITH_ARGS(                                                      \
        name,                                                                       \
        SSD1306_MULTIPLE(                                                           \
            uint8_t start_page_addr,                                                \
            uint8_t interval,                                                       \
            uint8_t end_page_addr                                                   \
        ),                                                                          \
        SSD1306_MULTIPLE(                                                           \
            (uint8_t) SSD1306_CTRL_COMMAND,                                         \
            (uint8_t) SSD1306_CMD_DUMMY_BYTE_00,                                    \
            (uint8_t) (cmd),                                                        \
            (uint8_t) (start_page_addr & SSD1306_PAGE_MASK),                        \
            (uint8_t) (interval & SSD1306_SCROLL_INTERVAL_MASK),                    \
            (uint8_t) (end_page_addr & SSD1306_PAGE_MASK),                          \
            (uint8_t) SSD1306_CMD_DUMMY_BYTE_00,                                    \
            (uint8_t) SSD1306_CMD_DUMMY_BYTE_FF                                     \
        )                                                                           \
    );

#define SSD1306_DEF_CMD_SCROLL_VH(name, cmd)                                        \
    SSD1306_DEF_CMD_WITH_ARGS(                                                      \
        name,                                                                       \
        SSD1306_MULTIPLE(                                                           \
            uint8_t start_page_addr,                                                \
            uint8_t interval,                                                       \
            uint8_t end_page_addr,                                                  \
            uint8_t vertical_offset                                                 \
        ),                                                                          \
        SSD1306_MULTIPLE(                                                           \
            (uint8_t) SSD1306_CTRL_COMMAND,                                         \
            (uint8_t) SSD1306_CMD_DUMMY_BYTE_00,                                    \
            (uint8_t) (cmd),                                                        \
            (uint8_t) (start_page_addr & SSD1306_PAGE_MASK),                        \
            (uint8_t) (interval & SSD1306_SCROLL_INTERVAL_MASK),                    \
            (uint8_t) (end_page_addr & SSD1306_PAGE_MASK),                          \
            (uint8_t) (vertical_offset & SSD1306_ROW_MASK)                          \
        )                                                                           \
    )

SSD1306_DEF_CMD_SCROLL_HOR(setup_hscroll_left, SSD1306_CMD_SCROLL_SETUP_H_LEFT);
SSD1306_DEF_CMD_SCROLL_HOR(setup_hscroll_right, SSD1306_CMD_SCROLL_SETUP_H_RIGHT);
SSD1306_DEF_CMD_SCROLL_VH(setup_vhscroll_left, SSD1306_CMD_SCROLL_SETUP_VH_LEFT);
SSD1306_DEF_CMD_SCROLL_VH(setup_vhscroll_right, SSD1306_CMD_SCROLL_SETUP_VH_RIGHT);
SSD1306_DEF_CMD(scroll_activate, SSD1306_CMD_SCROLL_ACTIVATE);
SSD1306_DEF_CMD(scroll_deactivate, SSD1306_CMD_SCROLL_DEACTIVATE);
SSD1306_DEF_CMD_WITH_ARGS(
        set_vscroll_area,
        SSD1306_MULTIPLE(uint8_t top_fixed_rows, uint8_t scroll_area_rows),
        SSD1306_MULTIPLE(
                (uint8_t) SSD1306_CTRL_COMMAND,
                (uint8_t) SSD1306_CMD_SCROLL_SETUP_V_AREA,
                (uint8_t) (top_fixed_rows & 0x3Fu),
                (uint8_t) (scroll_area_rows & SSD1306_COL_MASK)
        )
);

// Addressing settings commands

/**
 * Actually sends two commands!
 * SSD1306_CMD_ADDR_START_COL_LOWER (0x00) and SSD1306_CMD_ADDR_START_COL_LOWER (0x10)
 */
SSD1306_DEF_CMD_WITH_ARGS(
        page_addr_set_start_column,
        uint8_t col_address,
        SSD1306_MULTIPLE(
                (uint8_t) SSD1306_CTRL_CO_COMMAND,
                (((uint8_t) SSD1306_CMD_ADDR_START_COL_LOWER) | SSD1306_COL_LOWER(col_address)),
                (uint8_t) SSD1306_CTRL_COMMAND,
                (((uint8_t) SSD1306_CMD_ADDR_START_COL_HIGHER) | SSD1306_COL_HIGHER(col_address))
        )
);

SSD1306_DEF_CMD_WITH_ARG(set_addr_mode, SSD1306_CMD_ADDR_SET_ADDR_MODE, SSD1306AddrMode_t, addr_mode);
SSD1306_DEF_CMD_WITH_ARGS(
        vh_addr_set_column,
        SSD1306_MULTIPLE(uint8_t start_col_addr, uint8_t end_col_addr),
        SSD1306_MULTIPLE(
                (uint8_t) SSD1306_CTRL_COMMAND,
                (uint8_t) SSD1306_CMD_ADDR_SET_COLUMN,
                start_col_addr & SSD1306_COL_MASK,
                end_col_addr & SSD1306_COL_MASK
        )
);

SSD1306_DEF_CMD_WITH_ARGS(
        vh_addr_set_page,
        SSD1306_MULTIPLE(uint8_t start_page, uint8_t end_page),
       SSD1306_MULTIPLE(
               (uint8_t) SSD1306_CTRL_COMMAND,
               (uint8_t) SSD1306_CMD_ADDR_SET_PAGE,
               start_page & SSD1306_PAGE_MASK,
               end_page & SSD1306_PAGE_MASK
       )
);

SSD1306_DEF_CMD_WITH_ARGS(
        page_addr_set_page_start,
        uint8_t page,
        SSD1306_MULTIPLE(
                (uint8_t) SSD1306_CTRL_COMMAND,
                (uint8_t) SSD1306_CMD_ADDR_START_PAGE | (uint8_t) (page & SSD1306_PAGE_MASK)
        )
);

// Hardware configuration

SSD1306_DEF_CMD_WITH_ARGS(
        set_display_start_line,
        uint8_t line,
        SSD1306_MULTIPLE(
                (uint8_t) SSD1306_CTRL_COMMAND,
                (uint8_t) SSD1306_CMD_HW_SET_START_LINE | (line & SSD1306_ROW_MASK)
        )
)

SSD1306_DEF_CMD(segment_remap_off, SSD1306_CMD_HW_SEGMENT_REMAP_OFF);
SSD1306_DEF_CMD(segment_remap_on, SSD1306_CMD_HW_SEGMENT_REMAP_ON);
SSD1306_DEF_CMD_WITH_ARG(set_multiplex_ratio, SSD1306_CMD_HW_SET_MULTIPLEX_RATIO, uint8_t, value);
SSD1306_DEF_CMD(set_com_scan_direction_normal, SSD1306_CMD_HW_SET_COM_SCAN_DIR_NORMAL);
SSD1306_DEF_CMD(set_com_scan_direction_remap, SSD1306_CMD_HW_SET_COM_SCAN_DIR_REMAP);
SSD1306_DEF_CMD_WITH_ARG(set_display_offset, SSD1306_CMD_HW_SET_DISPLAY_OFFSET, uint8_t, offset);
SSD1306_DEF_CMD_WITH_ARG(
        set_com_pins_config,
        SSD1306_CMD_HW_SET_COM_PINS_CONFIG,
        SSD1306COMPinsConfig_t,
        config
);

// Timing and driving config

SSD1306_DEF_CMD_WITH_ARGS(
        set_oscillator_freq_and_divider,
        SSD1306_MULTIPLE(uint8_t osc_freq, uint8_t divider),
        SSD1306_MULTIPLE(
                (uint8_t) SSD1306_CTRL_COMMAND,
                (uint8_t) SSD1306_CMD_SET_DIV_RATIO_OSC_FREQ,
                ((osc_freq & 0xFu) << 0x4u) | (divider & 0xFu)
        )
);

SSD1306_DEF_CMD_WITH_ARGS(
        set_precharge_period,
        SSD1306_MULTIPLE(uint8_t phase_1, uint8_t phase_2),
        SSD1306_MULTIPLE(
                (uint8_t) SSD1306_CTRL_COMMAND,
                (uint8_t) SSD1306_CMD_SET_PRE_CHARGE_PERIOD,
                ((phase_2 & 0xFu) << 0x4u) | (phase_1 & 0xFu)
        )
);

SSD1306_DEF_CMD_WITH_ARG(
        set_vcomh_deselect_level,
        SSD1306_CMD_SET_VCOMH_DESELECT_LEVEL,
        SSD1306DeselectLevel_t,
        level
);

SSD1306_DEF_CMD(nop, SSD1306_CMD_NOP);

// Charge pump commands

SSD1306_DEF_CMD_WITH_ARG(setup_charge_pump, SSD1306_CMD_CHARGE_PUMP, SSD1306ChargePump_t, value);

#endif //THERMOMETER_SSD1306_H
