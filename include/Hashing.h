#ifndef HASHING_H_INCLUDED
#define HASHING_H_INCLUDED

#include "Caixa.h"

/* No da tabela de dispersao. As colisoes sao resolvidas por encadeamento
   (cada posicao da tabela e uma pequena lista ligada de caixas). */
typedef struct NoHash {
    Caixa *caixa;
    struct NoHash *prox;
} NoHash;

/* Tabela de dispersao das caixas, com chave = nome da caixa.
   Permite pesquisar uma caixa pelo nome de forma rapida. */
typedef struct {
    NoHash *tabela[TAMANHO_HASH];
    int totalCaixas;
} Hashing;

void   CriarHashing(Hashing *h);
int    FuncaoHash(char *nome);
void   InserirCaixa(Hashing *h, Caixa *c);
Caixa *PesquisarCaixa(Hashing *h, char *nome);
int    ObterTodasCaixas(Hashing *h, Caixa *vetor[], int maxCaixas); /* devolve quantas */
void   DestruirHashing(Hashing *h); /* destroi os nos e todas as caixas */

#endif // HASHING_H_INCLUDED
