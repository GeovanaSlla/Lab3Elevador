#include "elevador.h"

#define PENALIDADE 1
elevador_t vetorElevadores[NUM_ELEVADORES];


uint8_t getDestinoFinal(uint8_t elevador);

uint8_t penalidades(uint8_t elevador, uint8_t posChamada)
{
    uint8_t h = 0;
    if(vetorElevadores[elevador].mov == PARADO)
    {
        return h;
    }
    if(vetorElevadores[elevador].mov == SUBINDO)
    {
        uint8_t destinoFinal = getDestinoFinal(elevador);
        for (uint8_t i = vetorElevadores[elevador].pos; i < destinoFinal; i++)
        {
            if (vetorElevadores[elevador].paradas_subida[i] != NAO_DEVE_PARAR)
                h += PENALIDADE;
        }
        for (uint8_t i = destinoFinal; i > posChamada; i--)
        {
            if(vetorElevadores[elevador].paradas_descida[i] != NAO_DEVE_PARAR)
                h += PENALIDADE;
        }
    }
    else if (vetorElevadores[elevador].pos > posChamada)
    {
        for (uint8_t i = vetorElevadores[elevador].pos; i > posChamada; i--)
        {
            if (vetorElevadores[elevador].paradas_descida[i] != NAO_DEVE_PARAR)
                h += PENALIDADE;
        }
    }
    else
    {
        uint8_t destinoFinal = getDestinoFinal(elevador);
        for (uint8_t i = vetorElevadores[elevador].pos; i > destinoFinal; i--)
        {
            if (vetorElevadores[elevador].paradas_descida[i] != NAO_DEVE_PARAR)
                h += PENALIDADE;
        }
        for (uint8_t i = destinoFinal; i < posChamada; i++)
        {
            if(vetorElevadores[elevador].paradas_subida[i] != NAO_DEVE_PARAR)
                h += PENALIDADE;
        }
    }
    return h;
}

uint8_t avaliaElevador(uint8_t elevador, uint8_t posChamada)
{
    if(vetorElevadores[elevador].pos > posChamada)
    {
        if(vetorElevadores[elevador].mov == PARADO)
        {
            return vetorElevadores[elevador].pos - posChamada;
        }
        else if(vetorElevadores[elevador].mov == DESCENDO)
        {
            return vetorElevadores[elevador].pos - posChamada + penalidades(elevador, posChamada);
        }
        else
        {
            return 2 * getDestinoFinal(elevador) - vetorElevadores[elevador].pos - posChamada +
                                                        penalidades(elevador, posChamada);
        }
    }
    else if(vetorElevadores[elevador].pos == posChamada)
    {
        if(vetorElevadores[elevador].mov == PARADO)
        {
            return 0;
        }
        else if(vetorElevadores[elevador].mov == DESCENDO)
        {
            return 2*(posChamada - getDestinoFinal(elevador)) + penalidades(elevador, posChamada);
        }
        else
        {
            return 2*(getDestinoFinal(elevador) - posChamada) + penalidades(elevador, posChamada);
        }
    }
    else
    {
        if(vetorElevadores[elevador].mov == PARADO)
        {
            return posChamada - vetorElevadores[elevador].pos;
        }
        else if(vetorElevadores[elevador].mov == DESCENDO)
        {
            return vetorElevadores[elevador].pos + posChamada - 2 * getDestinoFinal(elevador)
                                                    + penalidades(elevador, posChamada);
        }
        else
        {
            uint8_t destinoFinal = getDestinoFinal(elevador);
            return destinoFinal - vetorElevadores[elevador].pos + ((destinoFinal > posChamada)?
                                                (destinoFinal - posChamada):(posChamada - destinoFinal));
        }
    }
}

uint8_t getDestinoFinal(uint8_t elevador)
{
    uint8_t i;
    if(vetorElevadores[elevador].mov == SUBINDO)
    {
        for(i = NUM_ANDARES; i > vetorElevadores[elevador].pos; i--)
        {
            if(vetorElevadores[elevador].paradas_subida[i] != NAO_DEVE_PARAR)
            {
                return i;
            }
        }
    }
    else if(vetorElevadores[elevador].mov == DESCENDO)
    {
        for(i = 0; i < vetorElevadores[elevador].pos; i++)
        {
            if(vetorElevadores[elevador].paradas_descida[i] != NAO_DEVE_PARAR)
            {
                return i;
            }
        }
    }
}