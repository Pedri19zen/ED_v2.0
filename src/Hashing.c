/* Hashing.c - tabela de dispersao que guarda as caixas (chave = nome). */

#include <string.h>
#include "Hashing.h"

/* Inicializa a tabela: todas as posicoes vazias. */
void CriarHashing(Hashing *h)
{
    int i;
    for (i = 0; i < TAMANHO_HASH; i++) h->tabela[i] = NULL;
    h->totalCaixas = 0;
}

/* Funcao de dispersao simples: soma dos codigos das letras, modulo o tamanho. */
int FuncaoHash(char *nome)
{
    int soma = 0, i;
    for (i = 0; nome[i] != '\0'; i++)
        soma += (unsigned char) nome[i];
    return soma % TAMANHO_HASH;
}

/* Insere uma caixa na tabela (a cabeca da lista da sua posicao). */
void InserirCaixa(Hashing *h, Caixa *c)
{
    int pos = FuncaoHash(c->nome);
    NoHash *novo = (NoHash *) malloc(sizeof(NoHash));
    novo->caixa = c;
    novo->prox = h->tabela[pos];
    h->tabela[pos] = novo;
    h->totalCaixas++;
}

/* Procura uma caixa pelo nome. Devolve a caixa ou NULL. */
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

/* Copia para 'vetor' os ponteiros de todas as caixas (ate maxCaixas).
   Devolve quantas caixas foram colocadas. Util para percorrer todas. */
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

/* Liberta todos os nos da tabela e tambem as caixas que eles guardam. */
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
