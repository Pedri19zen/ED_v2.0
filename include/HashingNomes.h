/**
 * @file HashingNomes.h
 * @brief Tabela de dispersao generica nome -> indice no array-mestre.
 *
 * Permite pesquisas O(1) media para clientes e produtos, em vez do
 * varrimento linear sobre 10.000+ entradas. As colisoes sao resolvidas
 * por encadeamento. Cada no guarda apenas um ponteiro para o nome
 * (que vive no array-mestre e nao muda), evitando copiar a string.
 */
#ifndef HASHING_NOMES_H_INCLUDED
#define HASHING_NOMES_H_INCLUDED

#include "Uteis.h"

#define TAMANHO_HASH_NOMES 1009   /**< Primo: ~10 entradas/bucket com 10k itens. */

/**
 * @brief No da tabela de dispersao nome -> indice.
 */
typedef struct NoHashNome {
    char *nome;                  /**< Aponta para o nome no array-mestre. */
    int   indice;                /**< Posicao no array-mestre. */
    struct NoHashNome *prox;     /**< Proximo no da cadeia. */
} NoHashNome;

/**
 * @brief Tabela de dispersao de nomes.
 */
typedef struct {
    NoHashNome *tabela[TAMANHO_HASH_NOMES];   /**< Cabecas dos buckets. */
    int totalEntradas;                         /**< Numero total de nomes. */
} HashingNomes;

/** @brief Inicializa a tabela vazia. */
void CriarHashingNomes(HashingNomes *h);
/** @brief Insere o par (nome, indice) na cabeca do bucket. */
void InserirNomeHash(HashingNomes *h, char *nome, int indice);
/**
 * @brief Pesquisa um nome na tabela.
 * @return Indice associado, ou -1 se nao existir.
 */
int  PesquisarNomeHash(HashingNomes *h, char *nome);
/** @brief Liberta todos os nos (os nomes em si vivem no array-mestre). */
void DestruirHashingNomes(HashingNomes *h);

#endif /* HASHING_NOMES_H_INCLUDED */
