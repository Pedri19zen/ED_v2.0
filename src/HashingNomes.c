/**
 * @file HashingNomes.c
 * @brief Tabela de dispersao nome -> indice (ver HashingNomes.h).
 */

#include <string.h>
#include "HashingNomes.h"

/** @brief Inicializa a tabela: todos os buckets vazios. */
void CriarHashingNomes(HashingNomes *h)
{
    int i;
    for (i = 0; i < TAMANHO_HASH_NOMES; i++) h->tabela[i] = NULL;
    h->totalEntradas = 0;
}

/**
 * @brief Funcao de dispersao "djb2"-like (multiplicador 31).
 * @param nome Cadeia a hashear.
 * @return Indice no array de buckets.
 */
static int FuncaoHashNome(char *nome)
{
    unsigned int s = 0;
    int i;
    for (i = 0; nome[i] != '\0'; i++)
        s = s * 31u + (unsigned char) nome[i];
    return (int) (s % TAMANHO_HASH_NOMES);
}

/** @brief Insere o par (nome, indice) na cabeca do bucket correspondente. */
void InserirNomeHash(HashingNomes *h, char *nome, int indice)
{
    int pos = FuncaoHashNome(nome);
    NoHashNome *novo = (NoHashNome *) malloc(sizeof(NoHashNome));
    novo->nome = nome;       /* guarda o ponteiro - nao copia */
    novo->indice = indice;
    novo->prox = h->tabela[pos];
    h->tabela[pos] = novo;
    h->totalEntradas++;
}

/** @brief Devolve o indice associado ao nome, ou -1 se nao existir. */
int PesquisarNomeHash(HashingNomes *h, char *nome)
{
    int pos = FuncaoHashNome(nome);
    NoHashNome *p = h->tabela[pos];
    while (p != NULL) {
        if (strcmp(p->nome, nome) == 0) return p->indice;
        p = p->prox;
    }
    return -1;
}

/** @brief Liberta todos os nos da tabela (os nomes vivem no array-mestre). */
void DestruirHashingNomes(HashingNomes *h)
{
    int i;
    NoHashNome *p, *seg;
    for (i = 0; i < TAMANHO_HASH_NOMES; i++) {
        p = h->tabela[i];
        while (p != NULL) {
            seg = p->prox;
            free(p);
            p = seg;
        }
        h->tabela[i] = NULL;
    }
    h->totalEntradas = 0;
}
