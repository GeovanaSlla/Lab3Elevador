#ifndef __UART_CONFIG_H__
#define __UART_CONFIG_H__

#include <stdint.h>
#include "cmsis_os2.h"

#define UART_RX_FLAG     0x1
#define UART_TX_FLAG     0x1 << 1

void     uartInit(osThreadId_t threadDelegada_aux);
//uint32_t uartRead(uint8_t* buf, uint32_t size);
uint8_t  uartGetChar();
void     uartPutChar(uint8_t c);
uint32_t uartSend(uint8_t* buf, uint32_t size);
#endif // __UART_CONFIG_H__
