/**
 * @file Pessoa.h
 * @brief Clientes do supermercado (array) com indice de nome e estado em runtime.
 */
#ifndef PESSOA_H_INCLUDED
#define PESSOA_H_INCLUDED

#include "Uteis.h"
#include "Produto.h"
#include "HashingNomes.h"

/**
 * @brief Um cliente do supermercado.
 *
 * Os campos booleanos controlam o estado durante a simulacao e evitam
 * estados invalidos (entrar duas vezes, sair sem ter entrado, etc.).
 */
typedef struct {
    int   codigo;               /**< Identificador (do ficheiro). */
    char  nome[MAX_NOME];       /**< Nome do cliente. */
    int   numProdutos;          /**< Artigos no carrinho. */
    float valorCarrinho;        /**< Soma dos precos dos artigos. */
    float precoMenorProduto;    /**< Preco do artigo mais barato (oferecido se preciso). */
    int   tempoAtendimento;     /**< Tempo total na caixa. */
    int   tempoRestante;        /**< Tempo que falta enquanto atendido. */
    int   tempoEspera;          /**< Tempo ja passado em fila. */
    int   tempoComprasRestante; /**< Tempo que falta para acabar as compras. */
    bool  ativo;                /**< Registo existe (gestao). */
    bool  dentroLoja;           /**< Esta dentro do supermercado. */
    bool  naFila;               /**< Esta em fila de uma caixa. */
    bool  foiOferecido;         /**< Ja recebeu um produto gratis. */
    char  caixaAtual[MAX_NOME]; /**< Nome da caixa onde esta ("" se nenhuma). */
} Cliente;

/**
 * @brief "Lista" de clientes (array) com indice de pesquisa O(1).
 */
typedef struct {
    Cliente v[MAX_CLIENTES];    /**< Array-mestre de clientes. */
    int total;                  /**< Numero de entradas usadas. */
    HashingNomes idxNome;       /**< Indice nome -> posicao no array. */
} ListaClientes;

/** @brief Inicializa a lista vazia. */
void CriarListaClientes(ListaClientes *L);
/** @brief Liberta a tabela idxNome (o array v[] vive na struct mae). */
void DestruirListaClientes(ListaClientes *L);

/**
 * @brief Adiciona um cliente registado (ainda fora da loja).
 * @return Indice ou -1 se cheio.
 */
int  AdicionarCliente(ListaClientes *L, char *nome, int numProdutos);

/**
 * @brief Pesquisa exata por nome (O(1) media via hashing).
 * @return Indice ou -1 se nao existe ou esta inactivo.
 */
int  PesquisarCliente(ListaClientes *L, char *nome);

/**
 * @brief Pesquisa case-insensitive + prefixo (fallback para varrimento linear).
 * @return Indice do primeiro cliente activo cujo nome comece por 'texto'.
 */
int  PesquisarClienteTolerante(ListaClientes *L, char *texto);

/** @brief Edita o numero de produtos do carrinho de um cliente. */
int  EditarCliente(ListaClientes *L, char *nome, int numProdutos);
/**
 * @brief Desactiva um cliente.
 * @return 1 ok, -1 se estiver dentro da loja, 0 nao encontrado.
 */
int  RemoverCliente(ListaClientes *L, char *nome);
/** @brief Lista clientes activos com paginacao. */
void ListarClientes(ListaClientes *L);
/** @brief Le a pool de clientes do ficheiro (formato "codigo TAB nome"). */
int  CarregarClientes(ListaClientes *L, char *ficheiro);
/** @brief Cria 'quantos' clientes ficticios "Cli#" para teste. */
void GerarClientesAleatorios(ListaClientes *L, int quantos, int maxProdutos);

/**
 * @brief Preenche o carrinho do cliente com produtos aleatorios.
 *
 * Calcula valorCarrinho, precoMenorProduto, tempoAtendimento e
 * tempoComprasRestante. Se 'prods' for NULL usa valores aleatorios.
 */
void PrepararCarrinho(Cliente *c, ListaProdutos *prods, int maxPreco, int tempoAtendProduto);

/** @brief Submenu interactivo de gestao de clientes. */
void MenuClientes(ListaClientes *L);

#endif /* PESSOA_H_INCLUDED */
