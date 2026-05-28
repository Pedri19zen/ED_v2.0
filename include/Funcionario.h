#ifndef FUNCIONARIO_H_INCLUDED
#define FUNCIONARIO_H_INCLUDED

#include "Uteis.h"

/* Um funcionario / operador de caixa.
   'dentroLoja' indica se esta ao servico; 'ocupado' se esta a operar uma caixa. */
typedef struct {
    char nome[MAX_NOME];
    bool ativo;             /* o registo existe */
    bool dentroLoja;        /* esta ao servico na loja */
    bool ocupado;           /* esta a operar uma caixa aberta */
    int  pessoasAtendidas;  /* estatisticas acumuladas */
    int  produtosVendidos;
} Funcionario;

/* "Lista" de funcionarios implementada como ARRAY, para acesso rapido. */
typedef struct {
    Funcionario v[MAX_FUNCIONARIOS];
    int total;
} ListaFuncionarios;

void CriarListaFuncionarios(ListaFuncionarios *L);
int  AdicionarFuncionario(ListaFuncionarios *L, char *nome);
int  PesquisarFuncionario(ListaFuncionarios *L, char *nome);
int  EditarFuncionario(ListaFuncionarios *L, char *nomeAntigo, char *nomeNovo);
int  RemoverFuncionario(ListaFuncionarios *L, char *nome); /* desativa (-1 se a operar) */
void ListarFuncionarios(ListaFuncionarios *L);
int  CarregarFuncionarios(ListaFuncionarios *L, char *ficheiro);
int  GravarFuncionarios(ListaFuncionarios *L, char *ficheiro);
Funcionario *ObterFuncionarioLivre(ListaFuncionarios *L);  /* ativo, na loja e !ocupado */
void MenuFuncionarios(ListaFuncionarios *L);

#endif // FUNCIONARIO_H_INCLUDED
