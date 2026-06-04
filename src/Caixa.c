/**
 * @file Caixa.c
 * @brief Operacoes proprias da caixa (criacao, lista de atendidos, destruicao).
 *
 * A logica de abrir/fechar/atender (que depende do contexto do
 * Supermercado) esta em Simulacao.c e Acoes.c.
 */

#include <string.h>
#include "Caixa.h"

/** @brief Cria uma caixa nova, sem operador e com a fila vazia. */
Caixa *CriarCaixa(char *nome, bool ativa)
{
    Caixa *c = (Caixa *) malloc(sizeof(Caixa));
    CopiarNome(c->nome, nome);
    c->ativa = ativa;
    c->aFechar = false;
    c->operador = NULL;
    CriarFila(&c->fila);
    c->aAtender = NULL;
    c->pessoasAtendidas = 0;
    c->produtosVendidos = 0;
    c->dinheiroFeito = 0;
    c->atendidos = NULL;
    return c;
}

/** @brief Acrescenta um nome a cabeca da lista de atendidos. */
void RegistarAtendido(Caixa *c, char *nomeCliente)
{
    NoNome *novo = (NoNome *) malloc(sizeof(NoNome));
    CopiarNome(novo->nome, nomeCliente);
    novo->prox = c->atendidos;   /* insere a cabeca da lista */
    c->atendidos = novo;
}

/** @brief Imprime "Caixa -- N cliente(s) atendido(s):" + um nome por linha. */
void ListarAtendidosCaixa(Caixa *c)
{
    NoNome *p;
    int total = 0;
    for (p = c->atendidos; p != NULL; p = p->prox) total++;
    printf("  %s -- %d cliente(s) atendido(s):\n", c->nome, total);
    if (c->atendidos == NULL) { printf("    (ninguem)\n"); return; }
    for (p = c->atendidos; p != NULL; p = p->prox)
        printf("    - %s\n", p->nome);
}

/** @brief Liberta a fila, a lista de atendidos e a propria caixa. */
void DestruirCaixa(Caixa *c)
{
    NoNome *p = c->atendidos, *seguinte;
    DestruirFila(&c->fila);
    while (p != NULL) { seguinte = p->prox; free(p); p = seguinte; }
    free(c);
}
