#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/uart.h"
#include "uart_config.h"
#include "driverleds.h"



extern const uint32_t SystemCoreClock;
osThreadId_t threadDelegada = NULL;

void UART0_IRQHandler();

void uartInit(osThreadId_t threadDelegada_aux)
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
  
  threadDelegada = threadDelegada_aux;
  
  uint32_t fifoRX, fifoTX;
  UARTFIFOLevelGet(UART0_BASE,&fifoTX,&fifoRX);

  //UARTFIFOEnable(UART0_BASE);
  UARTFIFODisable(UART0_BASE);
  //UARTFIFOLevelSet(UART0_BASE,UART_FIFO_TX4_8,UART_FIFO_RX1_8);
  //UARTFIFOLevelGet(UART0_BASE,&fifoTX,&fifoRX);

  UARTIntRegister(UART0_BASE,UART0_IRQHandler);//Habilitar interrup��es de tx e rx
  UARTIntEnable(UART0_BASE,UART_INT_TX|UART_INT_RX); 
  
}

/* uint32_t uartRead(uint8_t* buf, uint32_t size)
{
    int32_t c = UARTCharGetNonBlocking(UART0_BASE);
    uint32_t i = 0;

    while( (c != -1) && (i<size))
    {     
        buf[i++] = (uint8_t)c;
      
        c = UARTCharGetNonBlocking(UART0_BASE);        
    } while (c != -1);
    
    return i;
} */

uint8_t uartGetChar()
{
  int32_t c = UARTCharGetNonBlocking(UART0_BASE);
  if(c != -1)
    return (uint8_t)c;
  else
    return 0U;
}
void  uartPutChar(uint8_t c)
{
  UARTCharPut(UART0_BASE, c);
}

 /*uint32_t uartSend(uint8_t* buf, uint32_t size)
{
  //  int32_t c = UARTCharGetNonBlocking(UART0_BASE);
    uint32_t i = 0;
    uint8_t res = 0;
    for(i = 0; i<size; i++)
    {
      //res = UARTCharPutNonBlocking(UART0_BASE,buf[i]);   
      res =1;
UARTCharPut(UART0_BASE, buf[i]);
      //mudar pra blocking?
      if(res == 0)//não conseguiu colocar na fifo
        return size-i;//quanto faltou
    }  
     //res = UARTCharPutNonBlocking(UART0_BASE,'\r');  
     UARTCharPut(UART0_BASE, '\r');
    // res = res & UARTCharPutNonBlocking(UART0_BASE,'\n');  
     if(res == 0)
      return 1;

    return 0;
}*/

//Callback interrupção da Uart0
void UART0_IRQHandler()
{
  uint32_t uartStatus = UARTIntStatus(UART0_BASE,false);

  if(uartStatus & UART_INT_TX)
  {
    osThreadFlagsSet(threadDelegada,UART_TX_FLAG);
    UARTIntClear(UART0_BASE,UART_INT_TX);  
  }
  if(uartStatus & UART_INT_RX)
  {
    osThreadFlagsSet(threadDelegada,UART_RX_FLAG);
    UARTIntClear(UART0_BASE,UART_INT_RX);
  }
}
