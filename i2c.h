#ifndef THERMOMETER_I2C_H
#define THERMOMETER_I2C_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx.h"
#include "utils.h"

typedef enum I2CMode {
    I2C_MODE_TX = 0x0u,
    I2C_MODE_RX = 0x1u
} I2CMode_t;

void i2c_setup_timing_sm(I2C_TypeDef *i2c, uint32_t pclk1_freq);
void i2c_enable(I2C_TypeDef *i2c);
bool i2c_start(I2C_TypeDef *i2c, uint8_t address, I2CMode_t mode);
void i2c_send(I2C_TypeDef *i2c, const uint8_t *data, uint32_t data_count);
void i2c_stop(I2C_TypeDef *i2c);

// Convenience functions

static FORCE_INLINE void i2c_send_byte(I2C_TypeDef *i2c, uint8_t byte) {
    i2c_send(i2c, (const uint8_t[]) { byte }, 1);
}

#endif //THERMOMETER_I2C_H
