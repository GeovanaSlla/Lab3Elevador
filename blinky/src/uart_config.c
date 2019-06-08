#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "uart_config.h"
#include "driverleds.h"


extern uint32_t SystemCoreClock;

void UART0_IRQHandler();

void uartInit()
{
    //Inicializa Uart
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)); // Aguarda final da habilita��o
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)); // Aguarda final da habilita��o
  
  //GPIO da Uart
  GPIOPinConfigure(GPIO_PA0_U0RX);
  GPIOPinConfigure(GPIO_PA1_U0TX);
  GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);//TODO:ver
  UARTConfigSetExpClk(UART0_BASE, SystemCoreClock, 115200,(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));   
  
  
//Op��es de interrup��o
//UART_INT_9BIT - 9-bit Address Match interrupt
//UART_INT_OE - Overrun Error interrupt
//UART_INT_BE - Break Error interrupt
//UART_INT_PE - Parity Error interrupt
//UART_INT_FE - Framing Error interrupt
//UART_INT_RT - Receive Timeout interrupt
//UART_INT_TX - Transmit interrupt
//UART_INT_RX - Receive interrupt
//UART_INT_DSR - DSR interrupt
//UART_INT_DCD - DCD interrupt
//UART_INT_CTS - CTS interrupt
//UART_INT_RI - RI interrupt
  
  

UARTIntRegister(UART0_BASE,UART0_IRQHandler);//Habilitar interrup��es de tx e rx
UARTIntEnable(UART0_BASE,UART_INT_TX|UART_INT_RX); 
  
}
//
uint32_t uartRead(uint8_t* buf, uint32_t size)
{
    int32_t c = UARTCharGetNonBlocking(UART0_BASE);
    uint32_t i = 0;

    while( (c != -1) && (i<size))
    {
        buf[i++] = (uint8_t)c;
        c = UARTCharGetNonBlocking(UART0_BASE);        
    } while (c != -1);
    
    return i;
}

uint32_t uartSend(uint8_t* buf, uint32_t size)
{
  //  int32_t c = UARTCharGetNonBlocking(UART0_BASE);
    uint32_t i = 0;
    uint8_t res = 0;
    for(i = 0; i<size; i++)
    {
      res = UARTCharPutNonBlocking(UART0_BASE,buf[i]);     
      //mudar pra blocking?
      if(res == 0)//não conseguiu colocar na fifo
        return size-i;//quanto faltou
    }   
    return 0;
}

//Callback interrupção da Uart0
void UART0_IRQHandler()
{
  uint32_t uartStatus = UARTIntStatus(UART0_BASE,false);

  if(uartStatus & UART_INT_TX)
  {
    UARTIntClear(UART0_BASE,UART_INT_TX);  
  }
uint8_t state=0;
  if(uartStatus & UART_INT_RX)
  {
    //todo: delegar interrupt
    uint8_t buf_dummy[10];
    uint32_t size = uartRead(buf_dummy,10);
    state ^= LED2;
    LEDWrite(LED2, state);
    //todo: delegar interrupt
    
    UARTIntClear(UART0_BASE,UART_INT_RX);
  }
}
