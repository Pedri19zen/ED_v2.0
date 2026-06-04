/**
 * @file Fila.h
 * @brief Fila (FIFO) de clientes implementada como lista ligada.
 *
 * Cada no guarda apenas um ponteiro para o cliente (que vive no
 * array-mestre); destruir a fila NAO liberta os clientes.
 */
#ifndef FILA_H_INCLUDED
#define FILA_H_INCLUDED

#include "Pessoa.h"

/**
 * @brief No da lista ligada da fila.
 */
typedef struct NoFila {
    Cliente *cliente;          /**< Aponta para o cliente no array-mestre. */
    struct NoFila *prox;       /**< Proximo no da lista. */
} NoFila;

/**
 * @brief Fila (FIFO) com referencias para o inicio e o fim.
 */
typedef struct {
    NoFila *inicio;            /**< Primeiro no (ou NULL). */
    NoFila *fim;               /**< Ultimo no (ou NULL). */
    int tamanho;               /**< Numero de elementos. */
} Fila;

/** @brief Inicializa uma fila vazia. */
void     CriarFila(Fila *f);
/** @brief Devolve 1 se a fila estiver vazia. */
int      FilaVazia(Fila *f);
/** @brief Devolve o numero de elementos. */
int      TamanhoFila(Fila *f);
/** @brief Acrescenta um cliente ao fim da fila. */
void     EnfileirarCliente(Fila *f, Cliente *c);
/** @brief Espreita o cliente da frente sem o remover (NULL se vazia). */
Cliente *FrenteFila(Fila *f);
/** @brief Remove e devolve o cliente da frente (NULL se vazia). */
Cliente *DesenfileirarCliente(Fila *f);
/**
 * @brief Remove um cliente especifico da fila (usado para mover de caixa).
 * @return 1 se encontrou e removeu, 0 caso contrario.
 */
int      RemoverClienteDaFila(Fila *f, Cliente *c);
/** @brief Liberta todos os nos da fila (sem libertar os clientes). */
void     DestruirFila(Fila *f);

#endif /* FILA_H_INCLUDED */
