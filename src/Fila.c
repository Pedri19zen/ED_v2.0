/* Fila.c - fila de clientes (lista ligada) usada por cada caixa e pela
   lista de clientes que andam as compras. */

#include "Fila.h"

void CriarFila(Fila *f) { f->inicio = f->fim = NULL; f->tamanho = 0; }

int FilaVazia(Fila *f)  { return f->tamanho == 0; }

int TamanhoFila(Fila *f) { return f->tamanho; }

/* Acrescenta um cliente ao fim da fila. */
void EnfileirarCliente(Fila *f, Cliente *c)
{
    NoFila *novo = (NoFila *) malloc(sizeof(NoFila));
    novo->cliente = c;
    novo->prox = NULL;
    if (f->fim == NULL) f->inicio = f->fim = novo;   /* fila estava vazia */
    else { f->fim->prox = novo; f->fim = novo; }
    f->tamanho++;
}

/* Espreita o cliente que esta a frente, sem o retirar. */
Cliente *FrenteFila(Fila *f)
{
    return (f->inicio != NULL) ? f->inicio->cliente : NULL;
}

/* Retira e devolve o cliente do inicio da fila. */
Cliente *DesenfileirarCliente(Fila *f)
{
    NoFila *no;
    Cliente *c;
    if (f->inicio == NULL) return NULL;
    no = f->inicio;
    c = no->cliente;
    f->inicio = no->prox;
    if (f->inicio == NULL) f->fim = NULL;
    free(no);
    f->tamanho--;
    return c;
}

/* Remove um cliente especifico (usado para mudar de caixa / fechar caixa).
   Devolve 1 se o encontrou e removeu, 0 caso contrario. */
int RemoverClienteDaFila(Fila *f, Cliente *c)
{
    NoFila *atual = f->inicio, *anterior = NULL;
    while (atual != NULL) {
        if (atual->cliente == c) {
            if (anterior == NULL) f->inicio = atual->prox;
            else anterior->prox = atual->prox;
            if (atual == f->fim) f->fim = anterior;
            free(atual);
            f->tamanho--;
            return 1;
        }
        anterior = atual;
        atual = atual->prox;
    }
    return 0;
}

/* Liberta todos os nos da fila (mas nao os clientes). */
void DestruirFila(Fila *f)
{
    NoFila *atual = f->inicio, *seguinte;
    while (atual != NULL) {
        seguinte = atual->prox;
        free(atual);
        atual = seguinte;
    }
    f->inicio = f->fim = NULL;
    f->tamanho = 0;
}
