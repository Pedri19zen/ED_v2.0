#ifndef CAIXA_H_INCLUDED
#define CAIXA_H_INCLUDED

#include "Fila.h"
#include "Funcionario.h"

/* No simples (lista ligada) para guardar os nomes dos clientes atendidos. */
typedef struct NoNome {
    char nome[MAX_NOME];
    struct NoNome *prox;
} NoNome;

/* Uma caixa de atendimento.
   - fila:      clientes a espera de serem atendidos
   - aAtender:  cliente que esta a ser atendido neste momento (ou NULL)
   - aFechar:   marcada para fechar assim que terminar a fila (requisito 6)
   - atendidos: lista dos nomes ja atendidos por esta caixa (requisito 11) */
typedef struct {
    char nome[MAX_NOME];
    bool ativa;
    bool aFechar;
    Funcionario *operador;
    Fila fila;
    Cliente *aAtender;
    int pessoasAtendidas;
    int produtosVendidos;
    float dinheiroFeito;     /* total faturado por esta caixa (ja sem ofertas) */
    NoNome *atendidos;
} Caixa;

Caixa *CriarCaixa(char *nome, bool ativa);
void   RegistarAtendido(Caixa *c, char *nomeCliente);
void   ListarAtendidosCaixa(Caixa *c);
void   DestruirCaixa(Caixa *c);  /* liberta fila + lista de atendidos + a propria caixa */

#endif // CAIXA_H_INCLUDED
