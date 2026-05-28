#ifndef PRODUTO_H_INCLUDED
#define PRODUTO_H_INCLUDED

#include "Uteis.h"

/* Um produto do catalogo: nome e preco.
   'ativo' indica se o registo esta em uso (remover na gestao = desativar). */
typedef struct {
    char  nome[MAX_NOME];
    float preco;
    bool  ativo;
} Produto;

/* "Lista" de produtos implementada como ARRAY, para acesso rapido. */
typedef struct {
    Produto v[MAX_PRODUTOS];
    int total;
} ListaProdutos;

void CriarListaProdutos(ListaProdutos *L);
int  AdicionarProduto(ListaProdutos *L, char *nome, float preco); /* devolve indice ou -1 */
int  PesquisarProduto(ListaProdutos *L, char *nome);              /* devolve indice ou -1 */
int  EditarProduto(ListaProdutos *L, char *nome, float novoPreco);
int  RemoverProduto(ListaProdutos *L, char *nome);                /* desativa */
void ListarProdutos(ListaProdutos *L);
int  ProdutoAleatorio(ListaProdutos *L);                          /* indice de 1 ativo, ou -1 */
int  CarregarProdutos(ListaProdutos *L, char *ficheiro);
int  GravarProdutos(ListaProdutos *L, char *ficheiro);
void MenuProdutos(ListaProdutos *L);

#endif // PRODUTO_H_INCLUDED
