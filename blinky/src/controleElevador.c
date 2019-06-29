#include "controleElevador.h"
#include "interface_elevador.h"
#include "cmsis_os2.h" // CMSIS-RTOS

#define NUM_ELEVADORES 3

#define FLAG_EMBARQUE 0x01
#define FLAG_BT_INT 0x02
#define FLAG_PORTA 0x04
#define FLAG_POSICAO 0x08

#define NUM_FLAGS 4

#define VALOR_PORTA_ABERTA 0x10
#define VALOR_PORTA_FECHADA 0x20

#define TEMPO_PORTA_ABERTA 5000 // ms
const uint8_t elevador[NUM_ELEVADORES] = {ESQUERDA, CENTRAL, DIREITA};

osThreadId_t threadDistribuidoraID;
osThreadId_t threadControlaElevadorID[NUM_ELEVADORES];
osTimerId_t timerFechaPorta[NUM_ELEVADORES];
//Fila de mensagens definida na interface_elevador

void threadDistribuidora(void* arg)
{
    eventoElevador_t buffer_in;
    while(1)
    {
        osMessageQueueGet(queueEventosElevadorID, &buffer_in, NULL, osWaitForever);
        switch(buffer_in.tipo)
        {
            case PORTA_A:
                osThreadFlagsSet(threadControlaElevadorID[buffer_in.elevador], (VALOR_PORTA_ABERTA << NUM_FLAGS)|FLAG_PORTA);
                //osThreadFlagsSet(threadControlaElevadorID[buffer_in.elevador], FLAG_PORTA);
                break;
            case PORTA_F:
                osThreadFlagsSet(threadControlaElevadorID[buffer_in.elevador], (VALOR_PORTA_FECHADA << NUM_FLAGS)|FLAG_PORTA);
                //osThreadFlagsSet(threadControlaElevadorID[buffer_in.elevador], FLAG_PORTA);
                break;
            case BT_INT:
                osThreadFlagsSet(threadControlaElevadorID[buffer_in.elevador], (buffer_in.valor << NUM_FLAGS)|FLAG_BT_INT);
                //osThreadFlagsSet(threadControlaElevadorID[buffer_in.elevador], FLAG_BT_INT);
                break;
            case BT_EXT:
                uint8_t elevadorChamado = 0;
                uint8_t h = 255;
                for (uint8_t i = 0; i < NUM_ELEVADORES; i ++)
                {
                    uint8_t hnew = avaliaElevador(i, buffer_in.valor);
                    if(hnew < h)
                    {
                        h = hnew;
                        elevadorChamado = i;
                    }
                }
                osThreadFlagsSet(threadControlaElevadorID[elevadorChamado], (buffer_in.valor << NUM_FLAGS)|FLAG_EMBARQUE);
                //osThreadFlagsSet(threadControlaElevadorID[elevadorChamado], FLAG_EMBARQUE);
                break;
            case POSICAO:
                osThreadFlagsSet(threadControlaElevadorID[buffer_in.elevador], (buffer_in.valor << NUM_FLAGS)|FLAG_POSICAO);
                //osThreadFlagsSet(threadControlaElevadorID[buffer_in.elevador], FLAG_POSICAO);
                break;
            default:
                break;
        }
    }
}

void callBackFechaPorta(void* arg)
{
    comandoElevador_t comandoElevadorBuffer;
    comandoElevadorBuffer.elevador = (uint8_t) arg;
    comandoElevadorBuffer.comando = FECHA_PORTA;
    osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
}

void threadControlaElevador(void *arg)
{
    uint8_t elevador = (uint8_t) arg;
    comandoElevador_t comandoElevadorBuffer;
    if((timerFechaPorta[elevador] = osTimerNew(callBackFechaPorta, osTimerOnce, arg, NULL)) == NULL)
    {
        return;
    }
    while(1)
    {
        osThreadFlagsWait(FLAG_EMBARQUE|FLAG_BT_INT|FLAG_PORTA|FLAG_POSICAO, osFlagsWaitAny|osFlagsNoClear, osWaitForever);
        uint32_t flagValue = osThreadFlagsGet();
        uint8_t flagDisparada =  flagValue & 0x0F;
        uint32_t valor = (flagValue & 0xFFFFFFF0)>>NUM_FLAGS;
        osThreadFlagsClear(flagValue);
        switch (flagDisparada)
        {
            case FLAG_EMBARQUE:
                if(vetorElevadores[elevador].mov == SUBINDO)
                {
                    uint8_t andarFinal = getDestinoFinal(elevador);
                    if(andarFinal > valor)
                    {
                        vetorElevadores[elevador].paradas_descida[valor] = EMBARQUE;
                    }
                    else
                    {
                        vetorElevadores[elevador].paradas_subida[valor] = EMBARQUE;
                    }
                }
                else if(vetorElevadores[elevador].mov == DESCENDO)
                {
                    if(vetorElevadores[elevador].pos == 0)
                    {
                        comandoElevadorBuffer.elevador = elevador;
                        comandoElevadorBuffer.comando = FECHA_PORTA;
                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                    }
                    if(vetorElevadores[elevador].pos < valor)
                    {
                        vetorElevadores[elevador].paradas_subida[valor] = EMBARQUE;
                    }
                    else
                    {
                        vetorElevadores[elevador].paradas_subida[valor] = EMBARQUE;
                    }
                }
                else if(vetorElevadores[elevador].mov == PARADO)
                {
//                    if(vetorElevadores[elevador].pos == 0)
//                    {
//                        comandoElevadorBuffer.elevador = elevador;
//                        comandoElevadorBuffer.comando = FECHA_PORTA;
//                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
//                    }
                    if(vetorElevadores[elevador].pos < valor)
                    {
                        vetorElevadores[elevador].paradas_subida[valor] = EMBARQUE;
                        vetorElevadores[elevador].mov = SUBINDO;
                        comandoElevadorBuffer.comando = SOBE;
                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                    }
                    else if(vetorElevadores[elevador].pos > valor)
                    {
                        vetorElevadores[elevador].paradas_descida[valor] = EMBARQUE;
                        vetorElevadores[elevador].mov = DESCENDO;
                        comandoElevadorBuffer.comando = DESCE;
                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                    }
                    else
                    {
                        comandoElevadorBuffer.elevador = elevador;
                        comandoElevadorBuffer.comando = ABRE_PORTA;
                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever); 
                    }
                }
                break;
            case FLAG_BT_INT:
                if(vetorElevadores[elevador].pos < valor)
                {
                    vetorElevadores[elevador].paradas_subida[valor] = DESEMBARQUE;
                }
                else
                {
                    vetorElevadores[elevador].paradas_descida[valor] = DESEMBARQUE;
                }
                comandoElevadorBuffer.comando = ACENDE_BT;
                comandoElevadorBuffer.valor = valor;
                comandoElevadorBuffer.elevador = elevador;
                osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
//                if(vetorElevadores[elevador].pos == 0)
//                {
//                    comandoElevadorBuffer.comando = FECHA_PORTA;
//                    osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
//                }
                if(vetorElevadores[elevador].mov == PARADO)
                {
                    if(vetorElevadores[elevador].pos < valor)
                    {
                        vetorElevadores[elevador].mov = SUBINDO;
                        comandoElevadorBuffer.comando = SOBE;
                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                    }
                    else
                    {
                        vetorElevadores[elevador].mov = DESCENDO;
                        comandoElevadorBuffer.comando = DESCE;
                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                    }
                }
                break;
            case FLAG_PORTA:
                if(valor == VALOR_PORTA_ABERTA) //&& vetorElevadores[elevador].pos!=0)
                {
                    osTimerStart(timerFechaPorta[elevador], TEMPO_PORTA_ABERTA);
                }
                else
                {
                    if(vetorElevadores[elevador].mov == SUBINDO)
                    {
                        vetorElevadores[elevador].mov = PARADO;
                        for(uint8_t i = vetorElevadores[elevador].pos + 1; i <= NUM_ANDARES; i++)
                        {
                            if(vetorElevadores[elevador].paradas_subida[i] != NAO_DEVE_PARAR)
                            {
                                vetorElevadores[elevador].mov = SUBINDO;
                                comandoElevadorBuffer.comando = SOBE;
                                comandoElevadorBuffer.elevador = elevador;
                                osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                                break;
                            }
                        }
                        for(uint8_t i = vetorElevadores[elevador].pos - 1; i >= 1; i--)
                        {
                            if(vetorElevadores[elevador].paradas_descida[i] != NAO_DEVE_PARAR)
                            {
                                vetorElevadores[elevador].mov = DESCENDO;
                                comandoElevadorBuffer.comando = DESCE;
                                comandoElevadorBuffer.elevador = elevador;
                                osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                                break;
                            }
                        }
                        
                        break;
                    }
                    else if(vetorElevadores[elevador].mov == DESCENDO)
                    {
                        vetorElevadores[elevador].mov = PARADO;
                        for(uint8_t i = vetorElevadores[elevador].pos - 1; i >= 1; i--)
                        {
                            if(vetorElevadores[elevador].paradas_descida[i] != NAO_DEVE_PARAR)
                            {
                                vetorElevadores[elevador].mov = DESCENDO;
                                comandoElevadorBuffer.comando = DESCE;
                                comandoElevadorBuffer.elevador = elevador;
                                osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                                break;
                            }
                        }
                        for(uint8_t i = vetorElevadores[elevador].pos + 1; i <= NUM_ANDARES; i++)
                        {
                            if(vetorElevadores[elevador].paradas_subida[i] != NAO_DEVE_PARAR)
                            {
                                vetorElevadores[elevador].mov = SUBINDO;
                                comandoElevadorBuffer.comando = SOBE;
                                comandoElevadorBuffer.elevador = elevador;
                                osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                                break;
                            }
                        }
                        break;
                    }
                }
                break;
            case FLAG_POSICAO:
                vetorElevadores[elevador].pos = valor;
                if(vetorElevadores[elevador].mov == DESCENDO)
                {
                    if(vetorElevadores[elevador].paradas_descida[valor] != NAO_DEVE_PARAR)
                    {
                        comandoElevadorBuffer.comando = PARA;
                        comandoElevadorBuffer.elevador = elevador;
                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                        vetorElevadores[elevador].paradas_descida[valor] = NAO_DEVE_PARAR;
                        comandoElevadorBuffer.comando = ABRE_PORTA;
                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever); 
                        comandoElevadorBuffer.comando = APAGA_BT;
                        comandoElevadorBuffer.valor = valor;
                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                    }
                    break;
                }
                else if(vetorElevadores[elevador].mov == SUBINDO)
                {
                    if(vetorElevadores[elevador].paradas_subida[valor] != NAO_DEVE_PARAR)
                    {
                        comandoElevadorBuffer.comando = PARA;
                        comandoElevadorBuffer.elevador = elevador;
                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                        vetorElevadores[elevador].paradas_subida[valor] = NAO_DEVE_PARAR;
                        comandoElevadorBuffer.comando = ABRE_PORTA;
                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                        comandoElevadorBuffer.comando = APAGA_BT;
                        comandoElevadorBuffer.valor = valor;
                        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
                    }
                    break;
                }
                break;
            default:
                break;
        }
    }
}
uint8_t controleElevadorInit()
{
    comandoElevador_t comandoElevadorBuffer;
    if((threadDistribuidoraID = osThreadNew(threadDistribuidora, NULL, NULL)) == NULL)
    {
        return 0;
    }
    for (uint8_t i = 0; i < NUM_ELEVADORES; i++)
    {
        if((threadControlaElevadorID[i] = osThreadNew(threadControlaElevador, (void *)elevador[i] , NULL)) == NULL)
        {
            return 0;
        }
    }
    for(uint8_t i = 0; i < NUM_ELEVADORES; i++)
    {
        vetorElevadores[i].mov = PARADO;
        vetorElevadores[i].pos = 0;
        for(uint8_t j = 0; j <= NUM_ANDARES; j++)
        {
            vetorElevadores[i].paradas_descida[j] = NAO_DEVE_PARAR;
            vetorElevadores[i].paradas_subida[j] = NAO_DEVE_PARAR;
        }
        comandoElevadorBuffer.elevador = i;
        comandoElevadorBuffer.comando = INICIALIZA;
        osMessageQueuePut(queueComandosElevadorID, &comandoElevadorBuffer, 0, osWaitForever);
    }
    return 1;
}


