#ifndef STM_GPIO
#define STM_GPIO

/**
 * GPIO IO Modes
 **/

#define GPIO_IN_ANALOG 0b00 // Input analog
#define GPIO_IN_FLOATING 0b01 // Input floating
#define GPIO_IN_PU_PD 0b10 // Input with pull-up/pull-down

#define GPIO_OUT_PP 0b00 // Output push-pull
#define GPIO_OUT_OD 0b01 // Output Open-drain
#define GPIO_OUT_AF_PP 0b10 // Alternate function Push-pull output
#define GPIO_OUT_AF_OD 0b10 // Alternate function Open-drain output

#define GPIO_MODE_IN 0b00
#define GPIO_MODE_OUT_10MHZ 0b01
#define GPIO_MODE_OUT_2MHZ 0b10
#define GPIO_MODE_OUT_50MHZ 0b11

#endif
