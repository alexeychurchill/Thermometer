#include "power.h"
#include "stm32f1xx.h"
#include "rcc_setup.h"
#include "rtc.h"
#include "flash_settings.h"
#include "display.h"
#include "interfaces/buttons.h"

#define PWR_SLEEP_TIME_NONE                                 0u

static volatile uint32_t s_sleep_time = PWR_SLEEP_TIME_NONE;
static volatile PwrState_t s_pwr_state = PWR_STATE_RUNNING;
static volatile bool s_alarm_triggered = false;
static volatile bool s_sleep_allowed = true;
static volatile bool s_sleep_woke_up = false;

void pwr_schedule_sleep() {
    uint32_t sleep_timeout = settings_get_sleep();
    if (sleep_timeout == PWR_SLEEP_TIME_NONE) {
        s_sleep_time = PWR_SLEEP_TIME_NONE;
        return;
    }

    s_sleep_time = rtc_get_time() + sleep_timeout;
}

void pwr_sleep_tick() {
    // Actually, kinda of impossible state: device can be in the
    // PWR_STATE_SLEEPING starting from the pwr_sleep() function
    // and until code which handles RTC or EXTI wake up
    // ...
    // (potentially can happen if wake up will be caused nor by
    // RTC Alarm, nor by EXTI...
    if (s_pwr_state == PWR_STATE_SLEEPING) {
        return;
    }

    // Device had woken up to perform some "background" periodical work,
    // but button has been pressed, so device must be promoted to the fully
    // working mode woke up, i.e. all devices and peripherals for regular work
    // should be activated
    if (s_pwr_state == PWR_STATE_BG_WORK && hmi_btn_has_event()) {
        s_pwr_state = PWR_STATE_RUNNING;
        display_init();
        return;
    }

    uint32_t time = rtc_get_time();
    if (s_pwr_state == PWR_STATE_BG_WORK && !s_sleep_allowed) {
        // If PWR_STATE_BG_WORK, device must go to zzz when sleep will is allowed.
        // So, this means that program won't check sleep timeout at all:
        // it can be in past, in future, or, even (o, rly!?!?!?), has NONE value - device
        // doesn't care!
        return;
    } else if (s_pwr_state == PWR_STATE_RUNNING &&
            ( !s_sleep_allowed || (s_sleep_time == PWR_SLEEP_TIME_NONE) || (s_sleep_time > time) )
        ) {
        // If PWR_STATE_RUNNING, device must FIRST check if s_sleep_allowed,
        // THEN it should check if there are any sleep timeout and if it's
        // still in future
        return;
    }

    // Reset this anyway..
    s_sleep_time = PWR_SLEEP_TIME_NONE;
    // Zzzzzz...
    uint32_t wkup_period = settings_get_measure_period();
    uint32_t wkup_rtc_alarm = rtc_get_time() + wkup_period;
    rtc_set_alarm(wkup_rtc_alarm);
    pwr_sleep();
    // WAKE UP HERE!
    rtc_remove_alarm();
    s_sleep_woke_up = true;

    if (pwr_poll_alarm()) {
        // Woken up by RTC alarm interrupt -> background work
        s_pwr_state = PWR_STATE_BG_WORK;
        s_sleep_time = PWR_SLEEP_TIME_NONE;
    } else if (hmi_btn_has_event()) {
        // Woken up by button press (actually, EXTI under the hood)
        s_pwr_state = PWR_STATE_RUNNING;
        display_init();
        pwr_schedule_sleep();
    }
}

void pwr_sleep() {
    display_off();
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    PWR->CR &= ~PWR_CR_PDDS;
    PWR->CR |= PWR_CR_LPDS;
    s_pwr_state = PWR_STATE_SLEEPING;

    __WFI();

    rcc_setup_clocking();
}

bool pwr_poll_alarm() {
    bool alarm_triggered = s_alarm_triggered;
    s_alarm_triggered = false;
    return alarm_triggered;
}

bool pwr_poll_woke_up() {
    bool woke_up = s_sleep_woke_up;
    s_sleep_woke_up = false;
    return woke_up;
}

PwrState_t pwr_state() {
    return s_pwr_state;
}

void pwr_sleep_allow() {
    s_sleep_allowed = true;
}

void pwr_sleep_disallow() {
    s_sleep_allowed = false;
}
