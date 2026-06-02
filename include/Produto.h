#ifndef PRODUTO_H_INCLUDED
#define PRODUTO_H_INCLUDED

#include "Uteis.h"
#include "HashingNomes.h"

/* Um produto do catalogo. Os tempos (em segundos) sao especificos do produto:
   - tempoComprar: tempo que demora a apanha-lo na loja (fase de compras)
   - tempoPagar:   tempo que demora a passar pela caixa (atendimento)
   'ativo' indica se o registo esta em uso (remover na gestao = desativar). */
typedef struct {
    int   codigo;
    char  nome[MAX_NOME_PRODUTO];
    float preco;
    float tempoComprar;
    float tempoPagar;
    bool  ativo;
} Produto;

/* "Lista" de produtos implementada como ARRAY, para acesso rapido.
   'idxNome' indexa o nome -> indice no array (pesquisa O(1) media). */
typedef struct {
    Produto v[MAX_PRODUTOS];
    int total;
    HashingNomes idxNome;
} ListaProdutos;

void CriarListaProdutos(ListaProdutos *L);
void DestruirListaProdutos(ListaProdutos *L);     /* liberta a tabela idxNome */
int  AdicionarProduto(ListaProdutos *L, int codigo, char *nome,
                      float preco, float tempoComprar, float tempoPagar); /* indice ou -1 */
int  PesquisarProduto(ListaProdutos *L, char *nome);              /* devolve indice ou -1 */
int  EditarProduto(ListaProdutos *L, char *nome, float novoPreco);
int  RemoverProduto(ListaProdutos *L, char *nome);                /* desativa */
void ListarProdutos(ListaProdutos *L);
int  ProdutoAleatorio(ListaProdutos *L);                          /* indice de 1 ativo, ou -1 */
int  CarregarProdutos(ListaProdutos *L, char *ficheiro);
int  GravarProdutos(ListaProdutos *L, char *ficheiro);
void MenuProdutos(ListaProdutos *L);

#endif // PRODUTO_H_INCLUDED
