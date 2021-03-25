#include "i2c.h"

#define I2C_SM_HALF_PERIOD  5000u
#define I2C_FREQ_DIV_MHZ    1000000u
#define I2C_ADDR_SHIFT      0x1u

void i2c_setup_timing_sm(I2C_TypeDef *i2c, uint32_t pclk1_freq) {
    uint32_t pclk1_freq_mhz = pclk1_freq / I2C_FREQ_DIV_MHZ;
    i2c -> CR2 &= ~I2C_CR2_FREQ_Msk;
    i2c -> CR2 |= (I2C_CR2_FREQ_Msk & pclk1_freq_mhz);

    uint32_t pclk1_t_ns = 1000 / pclk1_freq_mhz;
    uint32_t clock_count = I2C_SM_HALF_PERIOD / pclk1_t_ns;
    i2c -> CCR &= ~I2C_CCR_CCR_Msk;
    i2c -> CCR |= (I2C_CCR_CCR_Msk & clock_count);

    i2c -> TRISE &= ~I2C_TRISE_TRISE_Msk;
    i2c -> TRISE |= (I2C_TRISE_TRISE_Msk & (pclk1_freq_mhz + 1u));
}

void i2c_enable(I2C_TypeDef *i2c) {
    i2c -> CR1 |= I2C_CR1_PE;
}

bool i2c_start(I2C_TypeDef *i2c, const uint8_t address, const I2CMode_t mode) {
    i2c -> CR1 |= I2C_CR1_START;
    while (!(i2c -> SR1 & I2C_SR1_SB)) ;
    (void) i2c -> SR1;

    i2c -> DR = (uint8_t) (address << I2C_ADDR_SHIFT) | mode;

    bool ack_received = true;
    while (!(i2c -> SR1 & I2C_SR1_ADDR)) {
        if (i2c -> SR1 & I2C_SR1_AF) {
            ack_received = false;
            break;
        }
    }

    (void) i2c -> SR1;
    (void) i2c -> SR2;

    return ack_received;
}

void i2c_send(I2C_TypeDef *i2c, const uint8_t *data, uint32_t data_count) {
    uint32_t index = 0;
    while (index < data_count) {
        while (!(i2c -> SR1 & I2C_SR1_TXE));
        i2c -> DR = data[index];
        index++;
    }
}

void i2c_stop(I2C_TypeDef *i2c) {
    while (!(i2c -> SR1 & I2C_SR1_BTF));

    i2c -> CR1 |= I2C_CR1_STOP;
    while (i2c -> CR1 & I2C_CR1_STOP);
}
