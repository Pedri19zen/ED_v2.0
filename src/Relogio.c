/* Relogio.c - relogio logico usado pela simulacao */

#include "Relogio.h"

/* Cria um relogio que avanca 'velocidade' segundos por passo (>= 1). */
Relogio *CriarRelogio(int velocidade)
{
    Relogio *R = (Relogio *) malloc(sizeof(Relogio));
    R->tempoAtual = 0;
    R->velocidade = (velocidade > 0) ? velocidade : 1;
    return R;
}

void DestruirRelogio(Relogio *R) { free(R); }

void AvancarRelogio(Relogio *R) { R->tempoAtual += R->velocidade; }

int GetTempo(Relogio *R) { return R->tempoAtual; }

void ReiniciarRelogio(Relogio *R) { R->tempoAtual = 0; }
