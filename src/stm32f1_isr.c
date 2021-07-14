#include "stm32f1xx.h"
#include "rtc.h"
#include "power.h"
#include "interfaces/buttons.h"

// All ISRs should go here...

void RTC_Alarm_IRQHandler() {
    // TODO: Change way of the RTC Alarm signalizing
    EXTI->PR |= EXTI_PR_PIF17;
    pwr_handle_wkup_isr(PWR_WKUP_SOURCE_ALARM);
    rtc_clear_alarm();
}

// Buttons ISRs

static inline void exti_handle_button(uint32_t pr_flag, HmiBtn_t button) {
    if (EXTI->PR & pr_flag) {
        if (!pwr_handle_wkup_isr(PWR_WKUP_SOURCE_BUTTON)) {
            hmi_btn_on_press(button);
        }
        EXTI->PR |= pr_flag;
    }
}

void EXTI3_IRQHandler(void) {
    exti_handle_button(EXTI_PR_PIF3, HMI_BTN_RIGHT);
}

void EXTI4_IRQHandler(void) {
    exti_handle_button(EXTI_PR_PIF4, HMI_BTN_ENTER);
}

void EXTI9_5_IRQHandler(void) {
    exti_handle_button(EXTI_PR_PIF5, HMI_BTN_LEFT);
}

void TIM4_IRQHandler(void) {
    if (TIM4->SR & TIM_SR_UIF) {
        hmi_btn_on_tick();
        TIM4->SR &= ~TIM_SR_UIF;
    }
}
