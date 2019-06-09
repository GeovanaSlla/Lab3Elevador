#ifndef _ELEVADOR_H_
#define _ELEVADOR_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define ABERTA false
#define FECHADA true
#define DEVE_PARAR true
#define NAO_DEVE_PARAR false
#define NUM_ELEVADORES 3
#define NUM_ANDARES 15

typedef enum movimento_e {PARADO, DESCENDO, SUBINDO} movimento_e;
typedef enum elevador_e {ESQUERDA = 1, CENTRAL, DIREITA} elevador_e;

typedef struct elevador_t
{
    uint8_t pos;
    movimento_e mov;
    bool porta;
    bool paradas_subida[NUM_ANDARES + 1];
    bool paradas_descida[NUM_ANDARES + 1];
} elevador_t;
#endif

extern elevador_t vetorElevadores[NUM_ELEVADORES];
uint8_t avaliaElevador(elevador_t* elevador, uint8_t posChamada);