/**
 * @file Produto.h
 * @brief Catalogo de produtos (array) com indice de pesquisa por nome.
 */
#ifndef PRODUTO_H_INCLUDED
#define PRODUTO_H_INCLUDED

#include "Uteis.h"
#include "HashingNomes.h"

/**
 * @brief Um produto do catalogo.
 *
 * Os tempos (em segundos) sao especificos do produto:
 * - tempoComprar: tempo na fase de "compras" (apanhar o artigo).
 * - tempoPagar:   tempo de passagem pela caixa.
 *
 * 'ativo' indica se o registo esta em uso (remover na gestao = desactivar).
 */
typedef struct {
    int   codigo;                       /**< Identificador unico do produto. */
    char  nome[MAX_NOME_PRODUTO];       /**< Nome (ate 127 caracteres). */
    float preco;                        /**< Preco em ]0, MAX_PRECO]. */
    float tempoComprar;                 /**< Segundos para apanhar na loja. */
    float tempoPagar;                   /**< Segundos para passar na caixa. */
    bool  ativo;                        /**< false = removido logicamente. */
} Produto;

/**
 * @brief "Lista" de produtos (array) com indice de pesquisa O(1).
 *
 * 'idxNome' mapeia nome -> indice no array para evitar varrimentos lineares.
 */
typedef struct {
    Produto v[MAX_PRODUTOS];   /**< Array de produtos. */
    int total;                 /**< Numero de entradas usadas. */
    HashingNomes idxNome;      /**< Indice nome -> posicao no array. */
} ListaProdutos;

/** @brief Inicializa a lista vazia. */
void CriarListaProdutos(ListaProdutos *L);
/** @brief Liberta os nos do indice de nomes. */
void DestruirListaProdutos(ListaProdutos *L);

/**
 * @brief Adiciona um produto ao catalogo.
 * @return Indice na lista ou -1 se cheio.
 */
int  AdicionarProduto(ListaProdutos *L, int codigo, char *nome,
                      float preco, float tempoComprar, float tempoPagar);

/**
 * @brief Procura um produto pelo nome (O(1) media via hashing).
 * @return Indice do produto activo ou -1 se nao existir.
 */
int  PesquisarProduto(ListaProdutos *L, char *nome);

/** @brief Altera o preco de um produto. @return 1 ok, 0 nao encontrado. */
int  EditarProduto(ListaProdutos *L, char *nome, float novoPreco);
/** @brief Desactiva (remove logicamente) um produto. @return 1/0. */
int  RemoverProduto(ListaProdutos *L, char *nome);
/** @brief Lista o catalogo com paginacao. */
void ListarProdutos(ListaProdutos *L);
/** @brief Devolve o indice de um produto activo ao acaso, ou -1. */
int  ProdutoAleatorio(ListaProdutos *L);
/** @brief Carrega o catalogo do ficheiro (TSV). @return 1 ok, 0 falha. */
int  CarregarProdutos(ListaProdutos *L, char *ficheiro);
/** @brief Grava o catalogo em formato TSV. */
int  GravarProdutos(ListaProdutos *L, char *ficheiro);
/** @brief Submenu interactivo de gestao do catalogo. */
void MenuProdutos(ListaProdutos *L);

#endif /* PRODUTO_H_INCLUDED */
