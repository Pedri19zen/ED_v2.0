#ifndef SUPERMERCADO_H_INCLUDED
#define SUPERMERCADO_H_INCLUDED

#include "Uteis.h"
#include "Relogio.h"
#include "Produto.h"
#include "Pessoa.h"
#include "Funcionario.h"
#include "Fila.h"
#include "Caixa.h"
#include "Hashing.h"

/* Estrutura central que liga tudo:
   - arrays-mestre: produtos, clientes e funcionarios (carregados uma vez)
   - emCompras: lista ligada de quem anda a fazer compras (ainda sem caixa)
   - caixas: tabela de dispersao com todas as caixas (HCaixas)
   - relogio e parametros de configuracao
   - estatisticas globais da simulacao */
typedef struct {
    char *nome;

    ListaProdutos     produtos;
    ListaClientes     clientes;
    ListaFuncionarios funcionarios;

    Fila    emCompras;   /* clientes dentro da loja, ainda a fazer compras */
    Hashing caixas;      /* todas as caixas, acessiveis pelo nome */
    Relogio *relogio;

    /* configuracao (lida de Configuracao.txt) */
    int MAX_ESPERA;
    int N_CAIXAS;
    int TEMPO_ATENDIMENTO_PRODUTO;
    int MAX_PRECO;
    int MAX_FILA;
    int MIN_FILA;

    int  cadenciaEntrada; /* probabilidade (%) de entrar um cliente por passo */
    bool aceitarEntradas; /* false = "porta fechada" (correr ate esvaziar) */
    bool verboso;         /* true = imprime os eventos passo a passo */

    /* estatisticas globais */
    int   totalAtendidos;
    int   totalProdutosVendidos;
    long  somaTemposEspera;
    int   produtosOferecidos;
    float custoOferecido;
    float totalDinheiro;        /* dinheiro total faturado por todas as caixas */
    int   caixasAbertasMax;
} Supermercado;

/* ---- ciclo de vida ---- */
Supermercado *CriarSupermercado(char *nome);
int  InicializarSupermercado(Supermercado *S);
void DestruirSupermercado(Supermercado *S);

/* ---- carregamento de ficheiros (requisito 1) ---- */
int CarregarConfiguracao(Supermercado *S, char *ficheiro);
int CarregarDados(Supermercado *S, char *ficheiro);

/* ---- simulacao ---- */
void ExecutarPasso(Supermercado *S);                       /* um "tick" */
void ExecutarSimulacao(Supermercado *S, int nPassos, int comPausa);
void CorrerAteEsvaziar(Supermercado *S, int comPausa);
int  SimulacaoTerminada(Supermercado *S);
bool LojaAberta(Supermercado *S);                          /* hora atual em horario? */

/* ---- acoes do gerente ---- */
int  AbrirNovaCaixa(Supermercado *S);                                   /* requisito 5 */
int  FecharCaixaImediato(Supermercado *S, char *nomeCaixa);             /* requisito 7 */
int  MoverClienteEntreCaixas(Supermercado *S, char *nomeCliente, char *nomeCaixa); /* req. 4 */
void PesquisarPessoa(Supermercado *S, char *nomeCliente);               /* requisito 8 */

/* ---- relatorios / medidas ---- */
void VerEstadoAtual(Supermercado *S);
void MedidasDesempenho(Supermercado *S);                   /* requisito 11 */
void MostrarMemoria(Supermercado *S);                      /* requisitos 9 e 10 */
long CalcularMemoriaUtilizada(Supermercado *S);
long CalcularMemoriaDesperdicada(Supermercado *S);
int  GravarDados(Supermercado *S, char *ficheiro);         /* requisito 2 */

#endif // SUPERMERCADO_H_INCLUDED
