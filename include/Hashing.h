/**
 * @file Hashing.h
 * @brief Tabela de dispersao que guarda as caixas (chave = nome da caixa).
 *
 * Colisoes resolvidas por encadeamento: cada posicao da tabela e' a cabeca
 * de uma pequena lista ligada de NoHash.
 */
#ifndef HASHING_H_INCLUDED
#define HASHING_H_INCLUDED

#include "Caixa.h"

/**
 * @brief No da tabela de dispersao das caixas.
 */
typedef struct NoHash {
    Caixa *caixa;              /**< Apontador para a caixa. */
    struct NoHash *prox;       /**< Proximo no na cadeia (colisao). */
} NoHash;

/**
 * @brief Tabela de dispersao das caixas (chave = nome).
 */
typedef struct {
    NoHash *tabela[TAMANHO_HASH];  /**< Cabecas de cada bucket. */
    int totalCaixas;               /**< Numero de caixas inseridas. */
} Hashing;

/** @brief Inicializa a tabela (todas as posicoes vazias). */
void   CriarHashing(Hashing *h);
/**
 * @brief Funcao de dispersao: soma dos codigos do nome modulo TAMANHO_HASH.
 * @param nome Nome a hashear.
 * @return Indice no array de buckets.
 */
int    FuncaoHash(char *nome);
/** @brief Insere uma caixa na tabela. */
void   InserirCaixa(Hashing *h, Caixa *c);
/** @brief Procura uma caixa pelo nome. @return Caixa ou NULL. */
Caixa *PesquisarCaixa(Hashing *h, char *nome);
/**
 * @brief Copia para 'vetor' os apontadores de todas as caixas.
 * @return Numero de caixas colocadas em 'vetor'.
 */
int    ObterTodasCaixas(Hashing *h, Caixa *vetor[], int maxCaixas);
/** @brief Liberta todos os nos da tabela E as caixas que guardam. */
void   DestruirHashing(Hashing *h);

#endif /* HASHING_H_INCLUDED */
