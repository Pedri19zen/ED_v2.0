#ifndef PESSOA_H_INCLUDED
#define PESSOA_H_INCLUDED

#include "Uteis.h"
#include "Produto.h"
#include "HashingNomes.h"

/* Um cliente do supermercado.
   Os campos booleanos controlam o estado do cliente durante a simulacao e
   evitam estados invalidos (entrar duas vezes, sair sem ter entrado, etc.). */
typedef struct {
    int   codigo;               /* identificador do cliente (do ficheiro) */
    char  nome[MAX_NOME];
    int   numProdutos;          /* artigos no carrinho */
    float valorCarrinho;        /* soma dos precos dos artigos */
    float precoMenorProduto;    /* preco do artigo mais barato (o que se oferece) */
    int   tempoAtendimento;     /* tempo total de passagem na caixa (soma de tempoPagar) */
    int   tempoRestante;        /* tempo que ainda falta enquanto e atendido */
    int   tempoEspera;          /* tempo ja passado em fila */
    int   tempoComprasRestante; /* tempo que falta para acabar as compras (soma de tempoComprar) */
    bool  ativo;                /* o registo existe (gestao) */
    bool  dentroLoja;           /* esta dentro do supermercado (insideStore) */
    bool  naFila;               /* esta numa fila de uma caixa  (inQueue) */
    bool  foiOferecido;         /* ja recebeu um produto gratis */
    char  caixaAtual[MAX_NOME]; /* nome da caixa onde esta ("" se nenhuma) */
} Cliente;

/* "Lista" de clientes implementada como ARRAY, para acesso rapido.
   Acompanha uma tabela de dispersao 'idxNome' (nome -> indice no array)
   para que PesquisarCliente seja O(1) em vez de varrer 12000 entradas. */
typedef struct {
    Cliente v[MAX_CLIENTES];
    int total;
    HashingNomes idxNome;
} ListaClientes;

void CriarListaClientes(ListaClientes *L);
void DestruirListaClientes(ListaClientes *L);     /* liberta a tabela idxNome */
int  AdicionarCliente(ListaClientes *L, char *nome, int numProdutos); /* indice ou -1 */
int  PesquisarCliente(ListaClientes *L, char *nome);                  /* indice ou -1 */
int  EditarCliente(ListaClientes *L, char *nome, int numProdutos);
int  RemoverCliente(ListaClientes *L, char *nome);  /* desativa (-1 se estiver na loja) */
void ListarClientes(ListaClientes *L);
int  CarregarClientes(ListaClientes *L, char *ficheiro);  /* le "codigo \t nome" */
void GerarClientesAleatorios(ListaClientes *L, int quantos, int maxProdutos);
/* Preenche o carrinho com produtos aleatorios e calcula valor e tempo. */
void PrepararCarrinho(Cliente *c, ListaProdutos *prods, int maxPreco, int tempoAtendProduto);
void MenuClientes(ListaClientes *L);

#endif // PESSOA_H_INCLUDED
