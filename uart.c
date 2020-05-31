#include "uart.h"
#include "stm32f1xx.h"

// Temporary!
#define BRR_DIV_MANTISSA (0x1D4)
#define BRR_DIV_FRACTION (0xC)

void uart1_init() {
    RCC -> APB2ENR |= RCC_APB2ENR_IOPAEN;

    GPIOA -> CRH &= ~GPIO_CRH_MODE9;
    GPIOA -> CRH &= ~GPIO_CRH_CNF9;
    GPIOA -> CRH |= (GPIO_CRH_MODE9_1 | GPIO_CRH_CNF9_1); // AF PP 2 MHz

    GPIOA -> CRH &= ~GPIO_CRH_MODE10;
    GPIOA -> CRH &= ~GPIO_CRH_CNF10;
    GPIOA -> CRH |= GPIO_CRH_CNF10_0; // IF

    RCC -> APB2ENR |= RCC_APB2ENR_USART1EN;

    // TODO: Add waitings for prevention of changing configuration
    // during transmission/receiption 

    USART1 -> CR1 = USART_CR1_UE; 
    USART1 -> CR1 &= ~USART_CR1_M;
    USART1 -> CR2 &= ~USART_CR2_STOP;

    uint32_t mantissa = BRR_DIV_MANTISSA << USART_BRR_DIV_Mantissa_Pos;
    uint32_t fraction = BRR_DIV_FRACTION << USART_BRR_DIV_Fraction_Pos;
    USART1 -> BRR = (mantissa | fraction);

    USART1 -> CR1 |= USART_CR1_TE;
}

void uart1_send_byte(const uint8_t data) {
    while (!(USART1 -> SR & USART_SR_TXE)) ;
    USART1 -> DR = data;
}

void uart1_send_str(char *data) {
    while (*data) {
        uart1_send_byte(*data++);
    }
}

void uart1_send_strn(char *data) {
    uart1_send_str(data);
    uart1_send_byte('\r');
    uart1_send_byte('\n');
}
