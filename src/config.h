#ifndef THERMOMETER_CONFIG_H
#define THERMOMETER_CONFIG_H

#include "stm32f1xx.h"
#include "drivers/ssd1306.h"

// Display
#define DISPLAY_I2C                         (I2C2)
#define DISPLAY_ADDR                        0x3Cu
#define DISPLAY_WIDTH                       0x80u
#define DISPLAY_HEIGHT                      0x40u
#define DISPLAY_SPACE_WIDTH                 0x08u
#define TEXT_SPACE_WIDTH                    0x08u

/**
 * DISPLAY_BUFFER_SIZE must be a product of
 * DISPLAY_WIDTH and DISPLAY_HEIGHT!
 */
#define DISPLAY_BUFFER_SIZE                 0x400u
#define DISPLAY_CONTRAST                    0x7F
#define DISPLAY_COM_PINS_CONFIG             (SSD1306_COM_PINS_CONFIG_ALTERNATIVE)

// Miscellaneous

#define POLL_TIMER (TIM3)
#define TSD_MEASURE_PERIOD_MS               450

#endif //THERMOMETER_CONFIG_H
