#ifndef FILA_H_INCLUDED
#define FILA_H_INCLUDED

#include "Pessoa.h"

/* No de uma fila ligada. Guarda apenas um PONTEIRO para o cliente (que vive
   no array-mestre); por isso destruir a fila NAO liberta os clientes. */
typedef struct NoFila {
    Cliente *cliente;
    struct NoFila *prox;
} NoFila;

/* Fila (FIFO) implementada como lista ligada com inicio e fim. */
typedef struct {
    NoFila *inicio;
    NoFila *fim;
    int tamanho;
} Fila;

void     CriarFila(Fila *f);
int      FilaVazia(Fila *f);
int      TamanhoFila(Fila *f);
void     EnfileirarCliente(Fila *f, Cliente *c);   /* poe no fim */
Cliente *FrenteFila(Fila *f);                       /* espreita o 1o (NULL se vazia) */
Cliente *DesenfileirarCliente(Fila *f);             /* remove e devolve o 1o */
int      RemoverClienteDaFila(Fila *f, Cliente *c); /* remove um cliente especifico */
void     DestruirFila(Fila *f);                     /* liberta todos os nos */

#endif // FILA_H_INCLUDED
