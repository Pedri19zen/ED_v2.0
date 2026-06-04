/**
 * @file Hashing.c
 * @brief Tabela de dispersao das caixas (chave = nome).
 */

#include <string.h>
#include "Hashing.h"

/** @brief Inicializa a tabela: todas as posicoes vazias. */
void CriarHashing(Hashing *h)
{
    int i;
    for (i = 0; i < TAMANHO_HASH; i++) h->tabela[i] = NULL;
    h->totalCaixas = 0;
}

/** @brief Soma dos codigos das letras modulo TAMANHO_HASH. */
int FuncaoHash(char *nome)
{
    int soma = 0, i;
    for (i = 0; nome[i] != '\0'; i++)
        soma += (unsigned char) nome[i];
    return soma % TAMANHO_HASH;
}

/** @brief Insere uma caixa na cabeca do bucket correspondente. */
void InserirCaixa(Hashing *h, Caixa *c)
{
    int pos = FuncaoHash(c->nome);
    NoHash *novo = (NoHash *) malloc(sizeof(NoHash));
    novo->caixa = c;
    novo->prox = h->tabela[pos];
    h->tabela[pos] = novo;
    h->totalCaixas++;
}

/** @brief Procura uma caixa pelo nome. */
Caixa *PesquisarCaixa(Hashing *h, char *nome)
{
    int pos = FuncaoHash(nome);
    NoHash *p = h->tabela[pos];
    while (p != NULL) {
        if (strcmp(p->caixa->nome, nome) == 0) return p->caixa;
        p = p->prox;
    }
    return NULL;
}

/** @brief Copia para 'vetor' os apontadores de todas as caixas. */
int ObterTodasCaixas(Hashing *h, Caixa *vetor[], int maxCaixas)
{
    int i, n = 0;
    NoHash *p;
    for (i = 0; i < TAMANHO_HASH; i++) {
        p = h->tabela[i];
        while (p != NULL && n < maxCaixas) {
            vetor[n++] = p->caixa;
            p = p->prox;
        }
    }
    return n;
}

/** @brief Liberta todos os nos da tabela e as caixas que eles guardam. */
void DestruirHashing(Hashing *h)
{
    int i;
    NoHash *p, *seguinte;
    for (i = 0; i < TAMANHO_HASH; i++) {
        p = h->tabela[i];
        while (p != NULL) {
            seguinte = p->prox;
            DestruirCaixa(p->caixa);
            free(p);
            p = seguinte;
        }
        h->tabela[i] = NULL;
    }
    h->totalCaixas = 0;
}
