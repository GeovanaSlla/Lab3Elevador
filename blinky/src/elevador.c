#include "elevador.h"

#define PENALIDADE 1
elevador_t vetorElevadores[NUM_ELEVADORES];

uint8_t getDestinoFinal(elevador_t* elevador);

uint8_t penalidades(elevador_t* elevador, uint8_t posChamada)
{
    uint8_t h = 0;
    if(elevador->mov == PARADO)
    {
        return h;
    }
    if(elevador->mov == SUBINDO)
    {
        uint8_t destinoFinal = getDestinoFinal(elevador);
        for (uint8_t i = elevador->pos; i < destinoFinal; i++)
        {
            if (elevador->paradas_subida[i] == DEVE_PARAR)
                h += PENALIDADE;
        }
        for (uint8_t i = destinoFinal; i > posChamada; i--)
        {
            if(elevador->paradas_descida[i] == DEVE_PARAR)
                h += PENALIDADE;
        }
    }
    else if (elevador->pos > posChamada)
    {
        for (uint8_t i = elevador->pos; i > posChamada; i--)
        {
            if (elevador->paradas_descida[i] == DEVE_PARAR)
                h += PENALIDADE;
        }
    }
    else
    {
        uint8_t destinoFinal = getDestinoFinal(elevador);
        for (uint8_t i = elevador->pos; i > destinoFinal; i--)
        {
            if (elevador->paradas_descida[i] == DEVE_PARAR)
                h += PENALIDADE;
        }
        for (uint8_t i = destinoFinal; i < posChamada; i++)
        {
            if(elevador->paradas_subida[i] == DEVE_PARAR)
                h += PENALIDADE;
        }
    }    
    return h;
}

uint8_t avaliaElevador(elevador_t* elevador, uint8_t posChamada)
{
    if(elevador->pos > posChamada)
    {
        if(elevador->mov == PARADO)
        {
            return elevador->pos - posChamada;
        }
        else if(elevador->mov == DESCENDO)
        {
            return elevador->pos - posChamada + penalidades(elevador, posChamada);
        }
        else
        {
            return 2 * getDestinoFinal(elevador) - elevador->pos - posChamada + 
                                                        penalidades(elevador, posChamada);
        }
    }
    else if(elevador->pos == posChamada)
    {
        if(elevador->mov == PARADO)
        {
            return 0;
        }
        else if(elevador->mov == DESCENDO)
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
        if(elevador->mov == PARADO)
        {
            return posChamada - elevador->pos;
        }
        else if(elevador->mov == DESCENDO)
        {
            return elevador->pos + posChamada - 2 * getDestinoFinal(elevador) 
                                                    + penalidades(elevador, posChamada);
        }
        else
        {
            uint8_t destinoFinal = getDestinoFinal(elevador);
            return destinoFinal - elevador->pos + ((destinoFinal > posChamada)?
                                                (destinoFinal - posChamada):(posChamada - destinoFinal));
        }
    }
    
}

uint8_t getDestinoFinal(elevador_t* elevador)
{
    uint8_t i;
    if(elevador->mov == SUBINDO)
    {
        for(i = NUM_ANDARES; i > elevador->pos; i--)
        {
            if(elevador->paradas_subida[i] == DEVE_PARAR)
            {
                return i;
            }
        }
    }
    else if(elevador->mov == DESCENDO)
    {
        for(i = 0; i < elevador->pos; i++)
        {
            if(elevador->paradas_descida[i] == DEVE_PARAR)
            {
                return i;
            }
        }
    }   
}