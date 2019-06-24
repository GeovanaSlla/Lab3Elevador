#include <stdint.h>
#include "cmsis_os2.h" // CMSIS-RTOS
#include "interface_elevador.h"
#include "elevador.h"
#include "uart_config.h"

#define CARACTER_FINALIZADOR '\r'
#define COMANDO_BUF_SIZE 3+1  //maiores comandos pro elevador tem 3 caracteres significativos e um caracter finalizador

#define RX_BUF_SIZE 32
#define TX_BUF_SIZE 32

uint8_t rxBuf[RX_BUF_SIZE];
uint8_t txBuf[TX_BUF_SIZE];
uint8_t rxWrIterator = 0;
uint8_t txRdIterator = 0;
uint8_t txWrIterator = 0;
//uint8_t rxMsgCompleta_flag = 0;

osThreadId_t threadInterfaceElevadorID;
osThreadId_t threadInterpretaEventoID;
osThreadId_t threadMontaComandoID;

osMessageQueueId_t queueEventosElevadorID;
osMessageQueueId_t queueComandosElevadorID;

void        threadInterfaceElevador(void* Arg);
void        threadInterpretaEvento(void *Arg);
void        threadMontaComando(void *Arg);

elevador_e  getElevador(uint8_t c);
tipo_e      getEvento(uint8_t c);
uint8_t     getElevadorChar(elevador_e elevador);
uint8_t     getComandoChar(comando_e comando);

uint8_t interfaceElevadorInit()
{
  threadInterfaceElevadorID = osThreadNew(threadInterfaceElevador, NULL, NULL);
  if(threadInterfaceElevadorID == NULL)
    return 0;

  threadInterpretaEventoID = osThreadNew(threadInterpretaEvento, NULL, NULL);
  if(threadInterpretaEventoID == NULL)
    return 0;

  threadMontaComandoID = osThreadNew(threadMontaComando, NULL, NULL);
  if(threadMontaComandoID == NULL)
    return 0;

  queueEventosElevadorID = osMessageQueueNew(NUMERO_MAX_EVENTOS, sizeof(eventoElevador_t), NULL);
  if(queueEventosElevadorID == NULL)
    return 0;

  queueComandosElevadorID = osMessageQueueNew(NUMERO_MAX_COMANDOS, sizeof(comandoElevador_t), NULL);
  if(queueComandosElevadorID == NULL)
    return 0;

  uartInit(threadInterfaceElevadorID);
  
//testes de envio de comando
/*   comandoElevador_t auxC;
  auxC.elevador = CENTRAL;
  auxC.comando = INICIALIZA;
  osMessageQueuePut(queueComandosElevadorID,&auxC,0,osWaitForever);
  auxC.comando = FECHA_PORTA;
  osMessageQueuePut(queueComandosElevadorID,&auxC,0,osWaitForever);
  auxC.comando = SOBE;
  osMessageQueuePut(queueComandosElevadorID,&auxC,0,osWaitForever);
  auxC.comando = ACENDE_BT;
  auxC.valor = 19;
  osMessageQueuePut(queueComandosElevadorID,&auxC,0,osWaitForever);
  auxC.comando = ACENDE_BT;
  auxC.valor = 1;
  osMessageQueuePut(queueComandosElevadorID,&auxC,0,osWaitForever); */
//
  return 1;
}

void threadInterfaceElevador(void *Arg)
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
          if((c > 0) && (c != '\n'))//ignorar \n
          {
             rxBuf[rxWrIterator++] = c;
             if(c == CARACTER_FINALIZADOR)
               osThreadFlagsSet(threadInterpretaEventoID,0x01);
          } 
        }while((c > 0) && (rxWrIterator < RX_BUF_SIZE));        
        
        if(rxWrIterator >= RX_BUF_SIZE) //ERRO - limpa estouro do buffer
        {
          rxWrIterator = 0;          
        }                        
      }
      if(UART_TX_FLAG & flag)
      {        
        osThreadFlagsClear(UART_TX_FLAG);

        if(txRdIterator > TX_BUF_SIZE)
          txRdIterator = 0;
        if(txWrIterator > TX_BUF_SIZE)
            txWrIterator = 0;   

        if(txRdIterator < txWrIterator)//tem caracter pra enviar
        {          
          uartPutChar(txBuf[txRdIterator++]);                    
        }
        else  //enviou todos ou erro -> resetar iteradores
        {
          txRdIterator = 0; 
          txWrIterator = 0;
        }                 
      }    
      if(COMANDO_FLAG & flag)
      {
        osThreadFlagsClear(COMANDO_FLAG);
      }
                  
  }
}

void threadInterpretaEvento(void *Arg)
{
  while (1)
  {
    osThreadFlagsWait(0x00000001, osFlagsWaitAny, osWaitForever);
  //-------------------------------------------------------------//
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
  //-------------------------------------------------------------//
    rxWrIterator = 0;
  }  
}

void threadMontaComando(void *Arg)       //ignora se o buffer de tx está cheio
{
  comandoElevador_t comando;
  uint8_t comandoBuf[COMANDO_BUF_SIZE];
  uint8_t comandoBufIterator;
  while(1)
  {
    osMessageQueueGet(queueComandosElevadorID,&comando,NULL,osWaitForever);
    comandoBufIterator = 0;
    
    comandoBuf[comandoBufIterator] = getElevadorChar(comando.elevador);
    if(comandoBuf[comandoBufIterator] != NULL)//elevador válido
    {
        comandoBuf[++comandoBufIterator] = getComandoChar(comando.comando);
        if(comandoBuf[comandoBufIterator] != NULL)//comando válido
        {
          if((comando.comando == ACENDE_BT) || (comando.comando == APAGA_BT)) // precisa da informação de botão
          {          
            comandoBuf[++comandoBufIterator] = comando.valor + BOTAO_TERREO;
            if((comandoBuf[comandoBufIterator] >= BOTAO_TERREO) && (comandoBuf[comandoBufIterator] <= BOTAO_ULTIMO_ANDAR))
            {
              comandoBuf[++comandoBufIterator] = CARACTER_FINALIZADOR;
              
              uint8_t i = 0;
              //if(txWrIterator >= TX_BUF_SIZE)
              for(i = 0; i <= comandoBufIterator; i++)
              {
                txBuf[txWrIterator++] = comandoBuf[i];
                if (txWrIterator > TX_BUF_SIZE)
                {
                  txWrIterator -= (i+1);//ignora comando
                  break;
                }
              }
              if(i == (comandoBufIterator + 1)) //conseguiu colocar no buffer
              {
                osThreadFlagsSet(threadInterfaceElevadorID,UART_TX_FLAG);
              }
            }
          }
          else
          {
            comandoBuf[++comandoBufIterator] = CARACTER_FINALIZADOR;
              
            uint8_t i = 0;
           // if(txWrIterator >= TX_BUF_SIZE)
            for(i = 0; i <= comandoBufIterator; i++)
            {
              txBuf[txWrIterator++] = comandoBuf[i];
              if (txWrIterator > TX_BUF_SIZE)
              {
                txWrIterator -= (i+1);//ignora comando
                break;
              }
            }
            if(i == (comandoBufIterator + 1)) //conseguiu colocar no buffer
            {
              osThreadFlagsSet(threadInterfaceElevadorID,UART_TX_FLAG);
            }
          }
          
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

uint8_t getElevadorChar(elevador_e elevador)
{
  switch (elevador)//define elevador
  {
    case ESQUERDA:
      return 'e';
      break;
    case CENTRAL:
      return 'c';
      break;
    case DIREITA:
      return 'd';
      break;

    default://TODO ERRO
      return NULL;
      break;
  }
} 

uint8_t getComandoChar(comando_e comando)
{
  switch (comando)
  {
    case INICIALIZA:
      return 'r';
      break;
    case ABRE_PORTA:
      return 'a';
      break;
    case FECHA_PORTA:
      return 'f';
      break;
    case SOBE:
      return 's';
      break;
    case DESCE:
      return 'd';
      break;
    case PARA:
      return 'p';
      break;
    case ACENDE_BT:
      return 'L';
      break;
    case APAGA_BT:
      return 'D';
      break;
          
    default:
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

