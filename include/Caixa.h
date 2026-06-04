/**
 * @file Caixa.h
 * @brief Caixa de atendimento: fila, operador e lista de clientes ja atendidos.
 */
#ifndef CAIXA_H_INCLUDED
#define CAIXA_H_INCLUDED

#include "Fila.h"
#include "Funcionario.h"

/**
 * @brief No simples para guardar os nomes dos clientes ja atendidos.
 */
typedef struct NoNome {
    char nome[MAX_NOME];       /**< Nome do cliente atendido. */
    struct NoNome *prox;       /**< Proximo no. */
} NoNome;

/**
 * @brief Uma caixa de atendimento.
 *
 * - 'fila':      clientes a espera de serem atendidos.
 * - 'aAtender':  cliente que esta a ser atendido neste momento (ou NULL).
 * - 'aFechar':   marcada para fechar assim que esvaziar (requisito 6).
 * - 'atendidos': lista dos nomes ja atendidos por esta caixa (requisito 11).
 */
typedef struct {
    char nome[MAX_NOME];       /**< Nome da caixa (ex.: "Caixa1"). */
    bool ativa;                /**< true se esta aberta. */
    bool aFechar;              /**< true se marcada para fechar. */
    Funcionario *operador;     /**< Operador actual (NULL se fechada). */
    Fila fila;                 /**< Fila FIFO de clientes em espera. */
    Cliente *aAtender;         /**< Cliente em atendimento (NULL se livre). */
    int pessoasAtendidas;      /**< Estatistica acumulada. */
    int produtosVendidos;      /**< Estatistica acumulada. */
    float dinheiroFeito;       /**< Total faturado (ja sem ofertas). */
    NoNome *atendidos;         /**< Lista ligada de nomes atendidos. */
} Caixa;

/**
 * @brief Cria uma caixa nova, sem operador e com a fila vazia.
 * @param nome  Nome a atribuir.
 * @param ativa Estado inicial.
 * @return Apontador para a caixa recem-alocada.
 */
Caixa *CriarCaixa(char *nome, bool ativa);

/**
 * @brief Acrescenta o nome de um cliente a lista de atendidos.
 * @param c Caixa que atendeu o cliente.
 * @param nomeCliente Nome a registar.
 */
void   RegistarAtendido(Caixa *c, char *nomeCliente);

/** @brief Imprime a lista de atendidos da caixa (um por linha). */
void   ListarAtendidosCaixa(Caixa *c);

/** @brief Liberta fila + lista de atendidos + a propria caixa. */
void   DestruirCaixa(Caixa *c);

#endif /* CAIXA_H_INCLUDED */
