#ifndef __INTERFACE_ELEVADOR_H__
#define __INTERFACE_ELEVADOR_H__

#include "elevador.h"
#include "cmsis_os2.h"
#define NUMERO_MAX_EVENTOS  5
#define NUMERO_MAX_COMANDOS 5
#define COMANDO_FLAG     0x1 << 2


typedef enum tipo_e {PORTA_A = 1, PORTA_F, BT_INT, BT_EXT, POSICAO} tipo_e;
typedef enum comando_e {INICIALIZA = 1, ABRE_PORTA, FECHA_PORTA, SOBE, DESCE, PARA, ACENDE_BT, APAGA_BT} comando_e;

typedef struct eventoElevador_t
{
  elevador_e elevador;
  tipo_e tipo;
  uint8_t valor;
} eventoElevador_t;

typedef struct comandoElevador_t
{
  elevador_e elevador;
  comando_e comando;
  uint8_t valor;
} comandoElevador_t;

extern osMessageQueueId_t queueEventosElevadorID;
extern osMessageQueueId_t queueComandosElevadorID;

uint8_t interfaceElevadorInit();

#endif // __INTERFACE_ELEVADOR_H__
