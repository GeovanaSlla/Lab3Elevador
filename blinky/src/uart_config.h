#ifndef __UART_CONFIG_H__
#define __UART_CONFIG_H__

#include <stdint.h>

void     uartInit();
uint32_t uartRead(uint8_t* buf, uint32_t size);
uint32_t uartSend(uint8_t* buf, uint32_t size);
#endif // __UART_CONFIG_H__
