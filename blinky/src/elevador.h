#ifndef _ELEVADOR_H_
#define _ELEVADOR_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define ABERTA false
#define FECHADA true
#define NUM_ELEVADORES 3
#define NUM_ANDARES 15
#define BOTAO_TERREO 'a'
#define BOTAO_ULTIMO_ANDAR (BOTAO_TERREO + NUM_ANDARES)

typedef enum movimento_e {PARADO, DESCENDO, SUBINDO} movimento_e;
typedef enum fluxo_e {DESEMBARQUE, NAO_DEVE_PARAR, EMBARQUE} fluxo_e;
typedef enum elevador_e{ESQUERDA = 0, CENTRAL, DIREITA} elevador_e;

typedef struct elevador_t
{
    uint8_t pos;
    movimento_e mov;
    fluxo_e paradas_subida[NUM_ANDARES + 1];
    fluxo_e paradas_descida[NUM_ANDARES + 1];
} elevador_t;
#endif

extern elevador_t vetorElevadores[NUM_ELEVADORES];
uint8_t avaliaElevador(uint8_t elevador, uint8_t posChamada);
