/**
 * @file Relogio.c
 * @brief Implementacao do relogio logico (ver Relogio.h).
 */

#include "Relogio.h"

/** @brief Cria um relogio com velocidade >= 1 (default 1 se <= 0). */
Relogio *CriarRelogio(int velocidade)
{
    Relogio *R = (Relogio *) malloc(sizeof(Relogio));
    R->tempoAtual = 0;
    R->velocidade = (velocidade > 0) ? velocidade : 1;
    return R;
}

/** @brief Liberta o relogio. */
void DestruirRelogio(Relogio *R) { free(R); }

/** @brief Avanca o relogio em 'velocidade' segundos. */
void AvancarRelogio(Relogio *R) { R->tempoAtual += R->velocidade; }

/** @brief Devolve o tempo actual (segundos). */
int GetTempo(Relogio *R) { return R->tempoAtual; }

/** @brief Repoe o tempo a zero. */
void ReiniciarRelogio(Relogio *R) { R->tempoAtual = 0; }
