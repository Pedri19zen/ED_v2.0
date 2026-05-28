#ifndef RELOGIO_H_INCLUDED
#define RELOGIO_H_INCLUDED

#include "Uteis.h"

/* Relogio logico da simulacao.
   Conta os segundos de simulacao decorridos; a cada "tick" (passo) o
   relogio avanca 'velocidade' segundos. Isto evita depender do tempo
   real do computador e torna a simulacao facil de seguir. */
typedef struct {
    int tempoAtual;   /* segundos de simulacao desde o inicio */
    int velocidade;   /* segundos que cada passo faz avancar */
} Relogio;

Relogio *CriarRelogio(int velocidade);
void DestruirRelogio(Relogio *R);
void AvancarRelogio(Relogio *R);   /* tempoAtual += velocidade */
int  GetTempo(Relogio *R);         /* devolve tempoAtual */
void ReiniciarRelogio(Relogio *R); /* poe tempoAtual a 0 */

#endif // RELOGIO_H_INCLUDED
