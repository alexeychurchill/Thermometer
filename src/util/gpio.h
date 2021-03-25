#ifndef STM_GPIO
#define STM_GPIO

#include <stdint.h>
#include "stm32f1xx.h"
#include "util/utils.h"

/**
 * GPIO IO Modes
 **/

#define GPIO_IN_ANALOG 0b00 // Input analog
#define GPIO_IN_FLOATING 0b01 // Input floating
#define GPIO_IN_PU_PD 0b10 // Input with pull-up/pull-down

#define GPIO_OUT_PP 0b00 // Output push-pull
#define GPIO_OUT_OD 0b01 // Output Open-drain
#define GPIO_OUT_AF_PP 0b10 // Alternate function Push-pull output
#define GPIO_OUT_AF_OD 0b11 // Alternate function Open-drain output

#define GPIO_MODE_IN 0b00
#define GPIO_MODE_OUT_10MHZ 0b01
#define GPIO_MODE_OUT_2MHZ 0b10
#define GPIO_MODE_OUT_50MHZ 0b11

/**
 * GPIO Pin values
 **/
#define GPIO_MIN_PIN 0x0
#define GPIO_MAX_PIN 0xF

#define GPIO_PIN_LOW_MAX 0x7

#define GPIO_PIN_IS_HIGH(pin) (pin > GPIO_PIN_LOW_MAX)

#define GPIO_PIN_BIT(pin) (0x1 << pin)

#define GPIO_PIN_CONF_RESET_MASK 0xF
#define GPIO_PIN_CONF_RESET_VALUE(shift) (~(GPIO_PIN_CONF_RESET_MASK << shift))

#define GPIO_PIN_CONF_PER_REG 0x8
#define GPIO_PIN_CONF_BITS_PER_PIN 0x4
#define GPIO_PIN_CONF_HIGH_SHIFT(pin) ((pin - GPIO_PIN_CONF_PER_REG) *\
    GPIO_PIN_CONF_BITS_PER_PIN)

#define GPIO_PIN_CONF_LOW_SHIFT(pin) (pin * GPIO_PIN_CONF_BITS_PER_PIN)

#define GPIO_CONF_CNF_SHIFT 0x2
#define GPIO_CONF_REG_VALUE(shift, cnf, mode) (((cnf << GPIO_CONF_CNF_SHIFT\
    ) | mode) << shift)

/**
 * GPIO Assertion helpers 
 **/
#define PIN_VALID(pin) (((pin) >= GPIO_MIN_PIN) && ((pin) <= GPIO_MAX_PIN))
#define PIN_INVALID(pin) (((pin) < GPIO_MIN_PIN) || ((pin) > GPIO_MAX_PIN))

/**
 * GPIO Procedures
 **/

void FORCE_INLINE gpio_setup(
    GPIO_TypeDef* gpio, uint8_t pin, uint8_t cnf, uint8_t mode
) {
    if (pin > GPIO_PIN_LOW_MAX) {
        uint8_t shift = GPIO_PIN_CONF_HIGH_SHIFT(pin);
        uint32_t reg_value = gpio -> CRH;
        reg_value &= GPIO_PIN_CONF_RESET_VALUE(shift);
        reg_value |= GPIO_CONF_REG_VALUE(shift, cnf, mode); 
        gpio -> CRH = reg_value;
    } else {
        uint8_t shift = GPIO_PIN_CONF_LOW_SHIFT(pin);
        uint32_t reg_value = gpio -> CRL; 
        reg_value &= GPIO_PIN_CONF_RESET_VALUE(shift);
        reg_value |= GPIO_CONF_REG_VALUE(shift, cnf, mode); 
        gpio -> CRL = reg_value;
    }
}

uint8_t FORCE_INLINE gpio_read(GPIO_TypeDef* gpio, uint8_t pin) {
    return ((gpio -> IDR) & GPIO_PIN_BIT(pin)) >> pin;
}

void FORCE_INLINE gpio_set(GPIO_TypeDef* gpio, uint8_t pin) { 
    gpio -> BSRR = GPIO_PIN_BIT(pin);
}

void FORCE_INLINE gpio_reset(GPIO_TypeDef* gpio, uint8_t pin) {
    gpio -> BRR = GPIO_PIN_BIT(pin);
}

#endif
