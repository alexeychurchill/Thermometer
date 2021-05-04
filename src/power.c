#include "power.h"
#include "stm32f1xx.h"
#include "rcc_setup.h"
#include "rtc.h"
#include "flash_settings.h"
#include "display.h"

#define PWR_SLEEP_TIME_NONE                                 0u

static volatile uint32_t s_sleep_time = PWR_SLEEP_TIME_NONE;

void pwr_schedule_sleep() {
    uint32_t sleep_timeout = settings_get_sleep();
    if (sleep_timeout == PWR_SLEEP_TIME_NONE) {
        s_sleep_time = PWR_SLEEP_TIME_NONE;
        return;
    }

    s_sleep_time = rtc_get_time() + sleep_timeout;
}

void pwr_sleep_tick() {
    uint32_t time = rtc_get_time();
    if ((s_sleep_time == PWR_SLEEP_TIME_NONE) || (s_sleep_time > time)) {
        return;
    }

    s_sleep_time = PWR_SLEEP_TIME_NONE;
    pwr_sleep();
}

void pwr_sleep() {
    display_off();
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    PWR->CR &= ~PWR_CR_PDDS;
    PWR->CR |= PWR_CR_LPDS;
    __WFI();
    rcc_setup_clocking();
    display_init();
}
