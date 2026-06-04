/**
 * @file Relogio.h
 * @brief Relogio logico da simulacao.
 *
 * Conta os segundos de simulacao decorridos. A cada "tick" (passo) o
 * relogio avanca 'velocidade' segundos. Isto torna a simulacao
 * independente do tempo real do computador e facil de seguir.
 */
#ifndef RELOGIO_H_INCLUDED
#define RELOGIO_H_INCLUDED

#include "Uteis.h"

/**
 * @brief Relogio logico que avanca em passos discretos.
 */
typedef struct {
    int tempoAtual;   /**< Segundos de simulacao desde o inicio. */
    int velocidade;   /**< Segundos que cada passo (tick) faz avancar. */
} Relogio;

/**
 * @brief Cria um relogio com a velocidade indicada.
 * @param velocidade Segundos por passo (>= 1).
 * @return Apontador para o relogio recem-alocado.
 */
Relogio *CriarRelogio(int velocidade);

/**
 * @brief Liberta o relogio.
 * @param R Relogio a libertar.
 */
void DestruirRelogio(Relogio *R);

/**
 * @brief Avanca o relogio um passo (tempoAtual += velocidade).
 * @param R Relogio.
 */
void AvancarRelogio(Relogio *R);

/**
 * @brief Devolve o tempo actual em segundos.
 * @param R Relogio.
 * @return Segundos de simulacao decorridos.
 */
int  GetTempo(Relogio *R);

/**
 * @brief Repoe tempoAtual a 0.
 * @param R Relogio.
 */
void ReiniciarRelogio(Relogio *R);

#endif /* RELOGIO_H_INCLUDED */
