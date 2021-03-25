#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart1_init();
void uart1_send_byte(const uint8_t);
void uart1_send_str(char*);
void uart1_send_strn(char*);
void uart1_send_number(uint32_t);

#endif
