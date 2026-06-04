/**
 * @file Uteis.h
 * @brief Constantes globais e utilitarios partilhados (aleatorios, leitura
 *        validada do teclado, historico CSV, cores ANSI e barra de fila).
 *
 * Este modulo nao depende de nenhum outro do projecto e e' incluido por
 * todos os restantes.
 */
#ifndef UTEIS_H_INCLUDED
#define UTEIS_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* ---- Tamanhos maximos dos arrays-mestre (dados carregados uma vez) ---- */
#define MAX_NOME           50     /**< Tamanho de nome de cliente/caixa/funcionario. */
#define MAX_NOME_PRODUTO   128    /**< Nomes de produtos sao mais longos. */
#define MAX_CLIENTES       12000  /**< Clientes registados (Clientes.txt). */
#define MAX_PRODUTOS       10000  /**< Produtos no catalogo. */
#define MAX_FUNCIONARIOS   200    /**< Operadores de caixa. */
#define MAX_CAIXAS         100    /**< Limite ao percorrer o hashing das caixas. */
#define CAPACIDADE_LOJA    100    /**< Clientes dentro da loja em simultaneo. */
#define TAMANHO_HASH       13     /**< Primo: tamanho da tabela das caixas. */

/* ---- Nomes dos ficheiros (entrada em data/, saida na raiz) ---- */
#define FICH_CONFIG        "data/Configuracao.txt"   /**< Parametros da simulacao. */
#define FICH_PRODUTOS      "data/Produtos.txt"       /**< Catalogo de produtos. */
#define FICH_FUNCIONARIOS  "data/Funcionarios.txt"   /**< Operadores de caixa. */
#define FICH_CLIENTES      "data/Clientes.txt"       /**< Pool de nomes de clientes. */
#define FICH_HISTORICO     "historico.csv"           /**< Log das acoes do utilizador. */
#define FICH_RESULTADO     "resultado.txt"           /**< Relatorio final. */

/**
 * @brief Devolve um inteiro aleatorio em [min, max].
 * @param min Limite inferior (inclusivo).
 * @param max Limite superior (inclusivo).
 * @return Valor aleatorio no intervalo.
 */
int Aleatorio(int min, int max);

/** @brief Esvazia o resto da linha que ficou no buffer de entrada. */
void  LimparBuffer(void);

/**
 * @brief Le uma linha do stdin e converte para inteiro.
 * @param txt Texto a mostrar como prompt.
 * @return O inteiro lido, ou -1 se a entrada nao for um numero.
 */
int   LerInteiro(char *txt);

/**
 * @brief Le um inteiro insistindo ate estar em [min, max].
 * @param txt Prompt a mostrar.
 * @param min Limite inferior aceitavel.
 * @param max Limite superior aceitavel.
 * @return O inteiro lido dentro do intervalo.
 */
int   LerOpcao(char *txt, int min, int max);

/**
 * @brief Le um numero decimal do stdin.
 * @param txt Prompt a mostrar.
 * @return O numero lido ou -1 se invalido.
 */
float LerFloat(char *txt);

/**
 * @brief Le uma linha para 'destino' (corta o '\\n' final).
 * @param txt Prompt a mostrar.
 * @param destino Buffer onde a string e' guardada.
 * @param tamanho Tamanho maximo do buffer.
 */
void  LerString(char *txt, char *destino, int tamanho);

/**
 * @brief Mostra "txt (S/N)" e devolve true se a resposta comecar por 'S'/'s'.
 * @param txt Pergunta a mostrar.
 * @return true se confirmado, false caso contrario.
 */
bool  Confirmar(char *txt);

/** @brief Converte um caracter para maiuscula (so a-z). */
char ToMaiscula(char x);

/**
 * @brief Copia de forma segura um nome para um buffer de tamanho MAX_NOME.
 * @param destino Buffer destino.
 * @param origem  String origem.
 */
void CopiarNome(char *destino, char *origem);

/** @brief Espera (aproximadamente) 'seconds' segundos em espera activa. */
void wait_segundos(int seconds);

/* ---- Teclado em tempo real (usado pela simulacao automatica) ---- */
/** @brief Pausa de 'ms' milissegundos sem gastar CPU. */
void DormirMs(int ms);
/** @brief Devolve 1 se houver uma tecla a espera (nao bloqueia). */
int  TeclaPressionada(void);
/** @brief Consome a(s) tecla(s) que estejam no buffer. */
void DescartarTecla(void);
/**
 * @brief Espera 'ms' ms; se uma tecla for premida durante a espera, devolve 1.
 * @param ms Tempo total a esperar em milissegundos.
 * @return 1 se uma tecla foi premida antes do tempo expirar, 0 caso contrario.
 */
int  EsperarOuTecla(int ms);

/* ---- Historico CSV das acoes do utilizador (requisito 3) ---- */
/** @brief Marca o inicio de uma sessao no historico CSV. */
void IniciarHistorico(void);

/**
 * @brief Acrescenta uma linha ao historico CSV.
 *
 * Formato: "data;accao;detalhe". O detalhe pode estar vazio (NULL) quando
 * a accao nao tem parametros associados.
 *
 * @param accao   Identificador curto da accao.
 * @param detalhe Parametro adicional (pode ser NULL).
 */
void RegistarHistorico(char *accao, char *detalhe);

/* ---- Cores ANSI (paleta 256, tons suaves) ---- */
#define COR_OK    "\x1b[38;5;108m"   /**< Verde-salva: fila <= 3. */
#define COR_WARN  "\x1b[38;5;179m"   /**< Mostarda:    fila <= 6. */
#define COR_ERR   "\x1b[38;5;174m"   /**< Coral:       fila > 6. */
#define COR_HDR   "\x1b[38;5;110m"   /**< Azul: cabecalhos. */
#define COR_DIM   "\x1b[38;5;244m"   /**< Cinza: separadores e [FECHADA]. */
#define COR_RESET "\x1b[0m"          /**< Repor a cor por omissao. */

/**
 * @brief Habilita ANSI Virtual Terminal e UTF-8 no terminal do Windows.
 *
 * Em Linux/macOS nao faz nada (as cores e o UTF-8 estao activos por omissao).
 */
void AtivarCoresTerminal(void);

/**
 * @brief Imprime "Fila: N pessoas [●●●·······]" colorida segundo o tamanho.
 *
 * @param n Tamanho da fila. Verde se n <= 3, amarelo se n <= 6, vermelho > 6.
 */
void ImprimirBarraFila(int n);

#endif /* UTEIS_H_INCLUDED */
