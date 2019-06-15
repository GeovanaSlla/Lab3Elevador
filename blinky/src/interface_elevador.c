#include <stdint.h>
#include "cmsis_os2.h" // CMSIS-RTOS
#include "interface_elevador.h"
#include "uart_config.h"

#define CARACTER_FINALIZADOR '\r'

#define RX_BUF_SIZE 32
#define TX_BUF_SIZE 32
uint8_t rxBuf[RX_BUF_SIZE];
uint8_t txBuf[TX_BUF_SIZE];
uint8_t rxWrIterator = 0;
uint8_t txWrIterator = 0;
uint8_t rxMsgCompleta_flag = 0;

osThreadId_t threadInterfaceElevadorID;

osMessageQueueId_t queueEventosElevadorID;
osMessageQueueId_t queueComandosElevadorID;

void        threadInterfaceElevador(void* Arg);
elevador_e  getElevador(uint8_t c);
tipo_e      getEvento(uint8_t c);
//uint8_t     putOnRXBuf();
void interpretaEvento();
uint8_t interfaceElevadorInit()
{
  threadInterfaceElevadorID = osThreadNew(threadInterfaceElevador, NULL, NULL);
  if(threadInterfaceElevadorID == NULL)
    return 0;

  queueEventosElevadorID = osMessageQueueNew(NUMERO_MAX_EVENTOS, sizeof(eventoElevador_t), NULL);
  if(queueEventosElevadorID == NULL)
    return 0;

  queueComandosElevadorID = osMessageQueueNew(NUMERO_MAX_COMANDOS, sizeof(comandoElevador_t), NULL);
  if(queueComandosElevadorID == NULL)
    return 0;

  uartInit(threadInterfaceElevadorID);
  uint8_t teste[]="dr";
  //uint8_t teste[]="teste";
  uartSend(teste,sizeof(teste));

  return 1;
}

void threadInterfaceElevador(void* Arg)
{
  while(1)
  {
      osThreadFlagsWait(0x0000000F, osFlagsNoClear, osWaitForever);
      uint32_t flag = osThreadFlagsGet();


      if(UART_RX_FLAG & flag)
      {
        osThreadFlagsClear(UART_RX_FLAG);
        uint8_t c;
        do
        {
          c = uartGetChar();
          if((c > 0) && (c != '\n'))
          {
             rxBuf[rxWrIterator++] = c;
             if(c == CARACTER_FINALIZADOR)
                rxMsgCompleta_flag = 1;
              //{
               // rxMsgCompleta_flag = 1;
                //interpretaEvento();
                //rxWrIterator = 0;
              //}
          } 
        }while((c > 0) && (rxWrIterator < RX_BUF_SIZE));
        
        if(rxMsgCompleta_flag)
        {
          interpretaEvento();
          rxMsgCompleta_flag = 0;
          rxWrIterator = 0;
        }
        
        if(rxWrIterator >= RX_BUF_SIZE) //ERRO - limpa estouro do buffer
        {
          rxWrIterator = 0;          
        }                        
      }
      if(UART_TX_FLAG & flag)
      {
        osThreadFlagsClear(UART_TX_FLAG);
      }    
      if(COMANDO_FLAG & flag)
      {
        osThreadFlagsClear(COMANDO_FLAG);
      }
                  
  }
}

void interpretaEvento()
{
        eventoElevador_t evento = {NULL,NULL,NULL};
        uint8_t i = 0;
        while((i <= rxWrIterator))//le todo o buffer
        {          
          do{
              evento.elevador = getElevador(rxBuf[i++]);               
          }while ((evento.elevador == NULL) && (i <= rxWrIterator));//procura um possível primeiro caracter; pode acabar achando um parametro

          if((evento.elevador != NULL) && (i <= rxWrIterator))//definiu um elevador e ainda tem caracteres     
          {
            evento.tipo = getEvento(rxBuf[i]);
            if((evento.tipo == PORTA_A) || (evento.tipo == PORTA_F))
            {
              if(rxBuf[++i] == CARACTER_FINALIZADOR)
              {
                //todo: colocar na fila de msg
                //rxMsgCompleta_flag--;
                osMessageQueuePut(queueEventosElevadorID,&evento,0,osWaitForever);
              }
              else//erro
              {
                //i--; 

                break;
              }
            }
            else if(evento.tipo == BT_INT) 
            {
              evento.valor = rxBuf[++i] - 'a';
              if(rxBuf[++i] == CARACTER_FINALIZADOR)
              {
                //rxMsgCompleta_flag--;
                if(/*(evento.valor >= 0) && */(evento.valor <= NUM_ANDARES))
                {
                  //todo: colocar na fila de msg
                  osMessageQueuePut(queueEventosElevadorID,&evento,0,osWaitForever);
                }
              }                  
              else//erro
              {
                
              break;
              }
            }
            else
            {
              if(evento.tipo == BT_EXT)
                i++;
             // i++;
              
              if((rxBuf[i] >= '0') && (rxBuf[i] <= '9'))//número do andar
              {
                evento.valor = rxBuf[i] - '0';
                i++;
                if((rxBuf[i] >= '0') && (rxBuf[i] <= '9'))
                {
                  evento.valor = (evento.valor *10) + (rxBuf[i] - '0');
                  i++;
                }                  
              }                                

              if(evento.tipo == BT_EXT)
              {
                  if((rxBuf[i] == 's') || (rxBuf[i] == 'd'))
                  {
                    i++;
                    if(rxBuf[i] == CARACTER_FINALIZADOR)
                    {                        
                        //rxMsgCompleta_flag--;
                        //todo: colocar na fila de msg
                        osMessageQueuePut(queueEventosElevadorID,&evento,0,osWaitForever);
                    }              
                    else//erro
                    {
                      break;
                    }
                    
                  }
                  else
                  {
                  i--;
                //erro
                break;
                  }
              }
              else
              {
                if(rxBuf[i] == CARACTER_FINALIZADOR)
                {
                  //rxMsgCompleta_flag--;
                  evento.tipo = POSICAO;
                  //todo: colocar na fila de msg
                  osMessageQueuePut(queueEventosElevadorID,&evento,0,osWaitForever);
                }
                else//erro
                {
                  break;
                }
                                  
              }
             // break;                                  
                                
            }
          } 

        }
}

elevador_e getElevador(uint8_t c)
{
  switch (c)//define elevador
  {
    case 'e':
      return ESQUERDA;
      break;
    case 'c':
      return CENTRAL;
      break;
    case 'd':
      return DIREITA;
      break;

    default://TODO ERRO
      return NULL;
      break;
  }
}

tipo_e getEvento(uint8_t c)
{
    switch (c)//define o evento
    {
      case 'A':
        return PORTA_A;                    
        break;
      case 'F':
        return PORTA_F;                    
        break;
      case 'I':
        return BT_INT;
        break;
      case 'E':
        return BT_EXT;
        break;              
      default:
        return NULL;
        break;
    }
}

/* uint8_t putOnRXBuf()
{
  static uint8_t iterator = 0;
  
  iterator = iterator + uartRead(&rxBuf[iterator],RX_BUF_SIZE-iterator);
    
  return iterator;
  
} */