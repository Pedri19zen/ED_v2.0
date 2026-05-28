/* Caixa.c - estrutura de uma caixa de atendimento.
   A logica de abrir/fechar/atender (que precisa do contexto do supermercado)
   esta em Supermercado.c; aqui ficam apenas as operacoes proprias da caixa. */

#include <string.h>
#include "Caixa.h"

/* Cria uma caixa nova, sem operador e com a fila vazia. */
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

/* Acrescenta o nome de um cliente a lista de atendidos da caixa. */
void RegistarAtendido(Caixa *c, char *nomeCliente)
{
    NoNome *novo = (NoNome *) malloc(sizeof(NoNome));
    CopiarNome(novo->nome, nomeCliente);
    novo->prox = c->atendidos;   /* insere a cabeca da lista */
    c->atendidos = novo;
}

void ListarAtendidosCaixa(Caixa *c)
{
    NoNome *p = c->atendidos;
    printf("  %s atendeu: ", c->nome);
    if (p == NULL) { printf("(ninguem)\n"); return; }
    while (p != NULL) { printf("%s ", p->nome); p = p->prox; }
    printf("\n");
}

/* Liberta a fila, a lista de atendidos e a propria caixa. */
void DestruirCaixa(Caixa *c)
{
    NoNome *p = c->atendidos, *seguinte;
    DestruirFila(&c->fila);
    while (p != NULL) { seguinte = p->prox; free(p); p = seguinte; }
    free(c);
}
