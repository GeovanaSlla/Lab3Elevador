#ifndef __INTERFACE_ELEVADOR_H__
#define __INTERFACE_ELEVADOR_H__

#include "elevador.h"

#define NUMERO_MAX_EVENTOS 5
#define COMANDO_FLAG     0x1 << 2


typedef enum tipo_e {PORTA_A = 1, PORTA_F, BT_INT, BT_EXT, POSICAO} tipo_e;

typedef struct eventoElevador_t
{
  elevador_e elevador;
  tipo_e tipo;
  uint8_t valor;
} eventoElevador_t;

uint8_t interfaceElevadorInit();

#endif // __INTERFACE_ELEVADOR_H__
