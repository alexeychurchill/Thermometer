#include "stm32f1xx.h"
#include "rtc.h"

// All ISRs should go here...

void RTC_Alarm_IRQHandler() {
    // TODO: Handle wakeup if sleeping & schedule measurement
    EXTI->PR |= EXTI_PR_PIF17;
    rtc_clear_alarm();
}
