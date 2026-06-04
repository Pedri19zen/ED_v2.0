/**
 * @file Supermercado.h
 * @brief Estrutura central do projecto: agrega arrays-mestre, filas, hashing
 *        e relogio, e expoe a API publica de simulacao, gestao e relatorios.
 *
 * Os modulos sao:
 *  - Supermercado.c -> ciclo de vida + I/O de ficheiros.
 *  - Simulacao.c    -> passos da simulacao (ExecutarPasso e amigos).
 *  - Acoes.c        -> acoes do gerente (abrir/fechar/mover caixa, pesquisar).
 *  - Relatorios.c   -> estado, medidas de desempenho, memoria, GravarDados.
 */
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

/**
 * @brief Estrutura central do projecto.
 *
 * Junta os arrays-mestre carregados uma vez dos ficheiros, as estruturas
 * dinamicas (fila de quem anda as compras e tabela de dispersao das caixas),
 * o relogio logico, os parametros de configuracao e as estatisticas globais.
 */
typedef struct {
    char *nome;                   /**< Nome do supermercado. */

    ListaProdutos     produtos;        /**< Catalogo + indice de pesquisa. */
    ListaClientes     clientes;        /**< Pool + indice de pesquisa. */
    ListaFuncionarios funcionarios;    /**< Operadores de caixa. */

    Fila    emCompras;            /**< Clientes a fazer compras (sem caixa ainda). */
    Hashing caixas;               /**< Todas as caixas, acessiveis pelo nome. */
    Relogio *relogio;             /**< Relogio logico da simulacao. */

    /* ---- configuracao (lida de Configuracao.txt) ---- */
    int MAX_ESPERA;                /**< Limiar para oferecer um produto. */
    int N_CAIXAS;                  /**< Numero maximo de caixas. */
    int TEMPO_ATENDIMENTO_PRODUTO; /**< Limite [2, este] de tempo por produto. */
    int MAX_PRECO;                 /**< Preco maximo de um produto. */
    int MAX_FILA;                  /**< Media de fila para abrir nova caixa. */
    int MIN_FILA;                  /**< Media de fila para fechar uma caixa. */
    int VELOCIDADE_RELOGIO;        /**< Segundos por tick. */
    int HORA_ABERTURA;             /**< 24h: hora a que a loja abre. */
    int HORA_FECHO;                /**< 24h: hora a que a loja fecha. */
    int CADENCIA_ENTRADA;          /**< Probabilidade (%) de entrada por tentativa. */
    int INTERVALO_AUTO_MS;         /**< Periodo (ms) entre frames do auto-display. */
    int TICKS_POR_FRAME;           /**< Passos avancados entre frames. */

    bool aceitarEntradas;          /**< false = porta fechada (atende ate esvaziar). */
    bool verboso;                  /**< true = imprime eventos passo a passo. */

    /* ---- estatisticas globais ---- */
    int   totalAtendidos;          /**< Total de clientes atendidos. */
    int   totalProdutosVendidos;   /**< Total de produtos vendidos. */
    long  somaTemposEspera;        /**< Soma dos tempos de espera. */
    int   produtosOferecidos;      /**< Numero de ofertas (req. qualidade). */
    float custoOferecido;          /**< Custo total das ofertas. */
    float totalDinheiro;           /**< Faturacao total. */
    int   caixasAbertasMax;        /**< Maximo de caixas abertas em simultaneo. */

    /* ---- contadores reiniciados a cada VerEstadoAtual ---- */
    int  entradasDesdeUpdate;      /**< Entradas desde o ultimo frame. */
    int  saidasDesdeUpdate;        /**< Saidas desde o ultimo frame. */
    char nomesEntradas[1024];      /**< Nomes acumulados (req. 2: quem entrou). */
    int  ofertasDesdeUpdate;       /**< Ofertas (MAX_ESPERA) desde o ultimo frame. */
    char nomesOfertas[1024];       /**< Linhas "Nome [ -X.XX EUR ]" acumuladas. */

    /* ---- metricas adicionais ---- */
    int  maxFilaObservada;         /**< Maior fila vista em qualquer caixa. */
    int  dentroLojaPico;           /**< Pico de clientes dentro da loja. */
    int  horaPico;                 /**< Segundos absolutos do pico. */
    int  entradasPorHora[24];      /**< Histograma de entradas. */
    int  saidasPorHora[24];        /**< Histograma de saidas. */
} Supermercado;

/* ---- ciclo de vida ---- */
/**
 * @brief Aloca e inicializa um Supermercado com defaults.
 * @param nome Nome do supermercado (sera copiado).
 * @return Apontador para a nova instancia.
 */
Supermercado *CriarSupermercado(char *nome);
/**
 * @brief Le todos os ficheiros e prepara o relogio.
 * @return 1 se ok (mesmo com ficheiros em falta, usa defaults).
 */
int  InicializarSupermercado(Supermercado *S);
/** @brief Liberta toda a memoria dinamica. */
void DestruirSupermercado(Supermercado *S);

/* ---- carregamento de ficheiros (requisito 1) ---- */
/** @brief Le pares "CHAVE valor" de Configuracao.txt. */
int CarregarConfiguracao(Supermercado *S, char *ficheiro);
/** @brief Le caixas + clientes iniciais de Dados.txt. */
int CarregarDados(Supermercado *S, char *ficheiro);

/* ---- primitivas partilhadas entre modulos ---- */
/** @brief Conta clientes activos dentro da loja. */
int    ContarDentroLoja(Supermercado *S);
/** @brief Conta caixas com ativa=true. */
int    ContarCaixasAbertas(Supermercado *S);
/**
 * @brief Escolhe a caixa aberta com a fila mais curta.
 * @param excluir Caixa a ignorar (NULL para nao excluir nenhuma).
 * @return Caixa ou NULL se nenhuma estiver disponivel.
 */
Caixa *EscolherMelhorCaixa(Supermercado *S, Caixa *excluir);

/* ---- simulacao ---- */
/** @brief Executa um tick (passo) completo da simulacao. */
void ExecutarPasso(Supermercado *S);
/** @brief Corre 'nPassos' passos consecutivos. */
void ExecutarSimulacao(Supermercado *S, int nPassos, int comPausa);
/** @brief Fecha a porta e atende ate a loja ficar vazia. */
void CorrerAteEsvaziar(Supermercado *S, int comPausa);
/** @brief Devolve 1 se nao houver ninguem dentro da loja. */
int  SimulacaoTerminada(Supermercado *S);
/** @brief true se a hora actual estiver em [HORA_ABERTURA, HORA_FECHO[. */
bool LojaAberta(Supermercado *S);
/** @brief Reinicia clientes, filas e estatisticas e poe o relogio a hora de abertura. */
void IniciarNovoDia(Supermercado *S);

/* ---- acoes do gerente ---- */
/** @brief Requisito 5: abre uma caixa (reabre fechada ou cria nova). */
int  AbrirNovaCaixa(Supermercado *S);
/** @brief Requisito 7: fecha caixa de imediato e redistribui clientes. */
int  FecharCaixaImediato(Supermercado *S, char *nomeCaixa);
/** @brief Requisito 4: muda um cliente em fila para outra caixa. */
int  MoverClienteEntreCaixas(Supermercado *S, char *nomeCliente, char *nomeCaixa);
/** @brief Requisito 8: diz onde esta uma pessoa (tolerante a maiusculas/prefixo). */
void PesquisarPessoa(Supermercado *S, char *nomeCliente);
/** @brief Requisito 11: lista os clientes atendidos por uma caixa. */
void ListarAtendidosPorCaixa(Supermercado *S, char *nomeCaixa);
/** @brief Resumo (multi-linha) de todas as caixas com barra colorida. */
void MostrarResumoCaixas(Supermercado *S);
/**
 * @brief Aceita "Caixa1" ou "1"; resolve para o nome canonico.
 * @param entrada Texto introduzido pelo utilizador.
 * @param destino Buffer onde escreve o nome (tamanho MAX_NOME).
 */
void ResolverNomeCaixa(char *entrada, char *destino);

/* ---- relatorios / medidas ---- */
/** @brief Imprime o estado da loja e das caixas (frame da simulacao). */
void VerEstadoAtual(Supermercado *S);
/** @brief Requisito 11: medidas de desempenho do sistema. */
void MedidasDesempenho(Supermercado *S);
/** @brief Mostra a memoria utilizada e desperdicada. */
void MostrarMemoria(Supermercado *S);
/** @brief Requisito 9: bytes totais alocados. */
long CalcularMemoriaUtilizada(Supermercado *S);
/** @brief Requisito 10: bytes reservados mas nao usados. */
long CalcularMemoriaDesperdicada(Supermercado *S);
/** @brief Requisito 2: grava o relatorio em ficheiro. */
int  GravarDados(Supermercado *S, char *ficheiro);

#endif /* SUPERMERCADO_H_INCLUDED */
