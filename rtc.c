#include "rtc.h"
#include "stm32f1xx.h"
#include "utils.h"

#define RTC_PRLH_SEC                                0x0u
#define RTC_PRLL_SEC                                0x7FFFu
#define RTC_CNTH_MSK                                0xFFFF0000u
#define RTC_CNTH_OFFSET                             0x10u
#define RTC_CNTL_MSK                                0x0000FFFFu

static FORCE_INLINE void __rtc_start() {
    RCC->BDCR |= RCC_BDCR_LSEON;
    while (!(RCC->BDCR & RCC_BDCR_LSERDY)) ;

    RCC->BDCR |= RCC_BDCR_RTCSEL_0;     // LSE
    RCC->BDCR |= RCC_BDCR_RTCEN;        // Start
}

static FORCE_INLINE void __rtc_wait_op_off() {
    while (!(RTC->CRL & RTC_CRL_RTOFF)) ;
}

static FORCE_INLINE void __rtc_wait_resync() {
    RTC->CRL &= ~RTC_CRL_RSF;
    while (!(RTC->CRL & RTC_CRL_RSF)) ;
}

static FORCE_INLINE void __rtc_cnf_start() {
    __rtc_wait_op_off();
    RTC->CRL |= RTC_CRL_CNF;
}

static FORCE_INLINE void __rtc_cnf_stop() {
    RTC->CRL &= ~RTC_CRL_CNF;
    __rtc_wait_op_off();
}

void rtc_init() {
    PWR->CR |= PWR_CR_DBP;

    if (RCC->BDCR & RCC_BDCR_RTCEN) {
        __rtc_wait_resync();
        return;
    }

    __rtc_start();
    __rtc_wait_resync();

    __rtc_cnf_start();

    RTC->PRLH = RTC_PRLH_SEC;
    RTC->PRLL = RTC_PRLL_SEC;

    RTC->CNTH = 0x0u;
    RTC->CNTL = 0x0u;

    __rtc_cnf_stop();
}

void rtc_set_time(uint32_t time) {
    __rtc_cnf_start();

    RTC->CNTH = (time & RTC_CNTH_MSK) >> RTC_CNTH_OFFSET;
    RTC->CNTL = time & RTC_CNTL_MSK;

    __rtc_cnf_stop();
}

const uint32_t rtc_get_time() {
    return (RTC->CNTH << RTC_CNTH_OFFSET) | (RTC->CNTL);
}
