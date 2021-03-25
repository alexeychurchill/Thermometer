#include "poll_timer.h"
#include "stm32f1xx.h"

#define TIMER_MS        (2000)
#define TIMER_MS_MUL    (2)

static TIM_TypeDef *timer;

void poll_timer_init(TIM_TypeDef *tim) {
    timer = tim;
}

void poll_timer_start(uint16_t duration) {
    if (timer -> CR1 & TIM_CR1_CEN) {
        return;
    }

    timer -> CR1 = 0x0;

    timer -> DIER = 0x0;
    timer -> PSC = SystemCoreClock / TIMER_MS - 1;
    timer -> ARR = duration * TIMER_MS_MUL;
    timer -> CNT = 0x0;
    timer -> SR = 0x0;
    timer -> CR1 = TIM_CR1_OPM;

    timer -> CR1 |= TIM_CR1_CEN;
}

bool poll_timer_is_running() {
    return (timer -> CR1 & TIM_CR1_CEN) ? true : false;
}
