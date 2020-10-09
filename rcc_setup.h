#ifndef THERMOMETER_RCC_SETUP_H
#define THERMOMETER_RCC_SETUP_H

#include "stm32f1xx.h"

static void rcc_setup_clocking() {
    RCC -> CR |= RCC_CR_HSEON;
    while (!(RCC -> CR & RCC_CR_HSERDY)) ;

    RCC -> CFGR |= RCC_CFGR_PLLSRC;
    RCC -> CFGR |= RCC_CFGR_PLLMULL9;
    RCC -> CFGR |= RCC_CFGR_PPRE1_DIV2;
    RCC -> CFGR |= RCC_CFGR_PPRE2_DIV1;

    FLASH -> ACR |= FLASH_ACR_LATENCY_2;

    RCC -> CR |= RCC_CR_PLLON;
    while (!(RCC -> CR & RCC_CR_PLLRDY)) ;

    RCC -> CFGR |= RCC_CFGR_SW_PLL;
    while (!(RCC -> CFGR & RCC_CFGR_SWS_PLL)) ;

    SystemCoreClockUpdate();
}

static void rcc_enable_peripherals_clocking() {
    RCC -> AHBENR |= RCC_AHBENR_DMA1EN;
    RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;
    RCC -> APB1ENR |= RCC_APB1ENR_TIM3EN;
    RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC -> APB2ENR |= RCC_APB2ENR_IOPCEN;
}

#endif //THERMOMETER_RCC_SETUP_H
