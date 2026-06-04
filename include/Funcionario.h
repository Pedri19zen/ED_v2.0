/**
 * @file Funcionario.h
 * @brief Operadores de caixa (array) e respectivo menu de gestao.
 */
#ifndef FUNCIONARIO_H_INCLUDED
#define FUNCIONARIO_H_INCLUDED

#include "Uteis.h"

/**
 * @brief Um funcionario / operador de caixa.
 *
 * 'dentroLoja' indica se esta ao servico; 'ocupado' se esta a operar
 * uma caixa aberta.
 */
typedef struct {
    int   codigo;            /**< Identificador (do ficheiro). */
    char  nome[MAX_NOME];    /**< Nome do funcionario. */
    bool  ativo;             /**< Registo existe. */
    bool  dentroLoja;        /**< Esta ao servico. */
    bool  ocupado;           /**< Esta a operar uma caixa aberta. */
    int   pessoasAtendidas;  /**< Total acumulado. */
    int   produtosVendidos;  /**< Total acumulado. */
    float dinheiroFeito;     /**< Total faturado pelo operador (sem ofertas). */
} Funcionario;

/**
 * @brief "Lista" de funcionarios implementada como array (pesquisa linear).
 *
 * Como MAX_FUNCIONARIOS e' relativamente pequeno (200) nao se justifica
 * o overhead de uma tabela de dispersao.
 */
typedef struct {
    Funcionario v[MAX_FUNCIONARIOS];  /**< Array-mestre. */
    int total;                        /**< Entradas usadas. */
} ListaFuncionarios;

/** @brief Inicializa a lista vazia. */
void CriarListaFuncionarios(ListaFuncionarios *L);
/** @brief Adiciona um funcionario novo. @return Indice ou -1. */
int  AdicionarFuncionario(ListaFuncionarios *L, char *nome);
/** @brief Procura um funcionario pelo nome. @return Indice ou -1. */
int  PesquisarFuncionario(ListaFuncionarios *L, char *nome);
/** @brief Renomeia um funcionario. @return 1/0. */
int  EditarFuncionario(ListaFuncionarios *L, char *nomeAntigo, char *nomeNovo);
/**
 * @brief Desactiva um funcionario.
 * @return 1 ok, -1 se estiver a operar uma caixa, 0 nao encontrado.
 */
int  RemoverFuncionario(ListaFuncionarios *L, char *nome);
/** @brief Lista os funcionarios activos. */
void ListarFuncionarios(ListaFuncionarios *L);
/** @brief Carrega de ficheiro (TSV ou linha simples). */
int  CarregarFuncionarios(ListaFuncionarios *L, char *ficheiro);
/** @brief Grava os funcionarios em ficheiro (um nome por linha). */
int  GravarFuncionarios(ListaFuncionarios *L, char *ficheiro);
/** @brief Devolve um funcionario disponivel ou NULL. */
Funcionario *ObterFuncionarioLivre(ListaFuncionarios *L);
/** @brief Submenu interactivo de gestao de funcionarios. */
void MenuFuncionarios(ListaFuncionarios *L);

#endif /* FUNCIONARIO_H_INCLUDED */
