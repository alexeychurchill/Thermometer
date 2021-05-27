#include "rtc.h"
#include "stm32f1xx.h"
#include "util/utils.h"

#define RTC_PRLH_SEC                                0x0u
#define RTC_PRLL_SEC                                0x7FFFu
#define RTC_CNTH_MSK                                0xFFFF0000u
#define RTC_CNTH_OFFSET                             0x10u
#define RTC_CNTL_MSK                                0x0000FFFFu

static FORCE_INLINE void enable_rtc() {
    RCC->BDCR |= RCC_BDCR_LSEON;
    while (!(RCC->BDCR & RCC_BDCR_LSERDY)) ;

    RCC->BDCR |= RCC_BDCR_RTCSEL_0;     // LSE
    RCC->BDCR |= RCC_BDCR_RTCEN;        // Start
}

static FORCE_INLINE void wait_op_off() {
    while (!(RTC->CRL & RTC_CRL_RTOFF)) ;
}

static FORCE_INLINE void wait_resync() {
    RTC->CRL &= ~RTC_CRL_RSF;
    while (!(RTC->CRL & RTC_CRL_RSF)) ;
}

static FORCE_INLINE void cnf_start() {
    wait_op_off();
    RTC->CRL |= RTC_CRL_CNF;
}

static FORCE_INLINE void cnf_stop() {
    RTC->CRL &= ~RTC_CRL_CNF;
    wait_op_off();
}

void rtc_init() {
    PWR->CR |= PWR_CR_DBP;

    if (RCC->BDCR & RCC_BDCR_RTCEN) {
        wait_resync();
        return;
    }

    enable_rtc();
    wait_resync();

    cnf_start();

    RTC->PRLH = RTC_PRLH_SEC;
    RTC->PRLL = RTC_PRLL_SEC;

    RTC->CNTH = 0x0u;
    RTC->CNTL = 0x0u;

    cnf_stop();
}

void rtc_set_time(uint32_t time) {
    cnf_start();

    RTC->CNTH = (time & RTC_CNTH_MSK) >> RTC_CNTH_OFFSET;
    RTC->CNTL = time & RTC_CNTL_MSK;

    cnf_stop();
}

uint32_t rtc_get_time() {
    return (RTC->CNTH << RTC_CNTH_OFFSET) | (RTC->CNTL);
}

void rtc_set_alarm(uint32_t time) {
    cnf_start();

    RTC->ALRL = time & 0x0000FFFFu;
    RTC->ALRH = (time & 0xFFFF0000u) >> 16u;
    RTC->CRH |= RTC_CRH_ALRIE;

    cnf_stop();
}

void rtc_remove_alarm() {
    cnf_start();

    RTC->CRH &= ~RTC_CRH_ALRIE;
    RTC->ALRH = RTC_ALRH_RTC_ALR;
    RTC->ALRL = RTC_ALRL_RTC_ALR;

    cnf_stop();
}

bool rtc_has_alarm() {
    return RTC->CRL & RTC_CRL_ALRF;
}

void rtc_clear_alarm() {
    cnf_start();
    RTC->CRL &= ~RTC_CRL_ALRF;
    cnf_stop();
}
