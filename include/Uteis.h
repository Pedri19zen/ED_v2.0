#ifndef UTEIS_H_INCLUDED
#define UTEIS_H_INCLUDED

/* =====================================================================
   Uteis.h - constantes globais do projeto e funcoes auxiliares usadas
   por (quase) todos os modulos: numeros aleatorios, leitura validada do
   teclado e registo do historico de acoes do utilizador.
   ===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* ---- Tamanhos maximos dos arrays-mestre (dados carregados uma vez) ---- */
#define MAX_NOME           50     /* nome de cliente, caixa ou funcionario */
#define MAX_NOME_PRODUTO   128    /* nomes de produtos sao mais longos */
#define MAX_CLIENTES       12000  /* clientes registados (suporta Clientes.txt + Dados.txt) */
#define MAX_PRODUTOS       10000  /* produtos no catalogo (suporta o ficheiro completo) */
#define MAX_FUNCIONARIOS   200    /* operadores de caixa (suporta Funcionarios.txt) */
#define MAX_CAIXAS         100    /* limite ao percorrer o hashing das caixas */
#define CAPACIDADE_LOJA    100    /* clientes dentro da loja em simultaneo */
#define TAMANHO_HASH       13     /* tamanho (primo) da tabela das caixas */

/* ---- Nomes dos ficheiros ---- */
/* Os ficheiros de ENTRADA estao na pasta data/; os de SAIDA sao criados na
   pasta onde o programa e executado (a raiz do projeto). */
#define FICH_CONFIG        "data/Configuracao.txt"
#define FICH_DADOS         "data/Dados.txt"
#define FICH_PRODUTOS      "data/Produtos.txt"
#define FICH_FUNCIONARIOS  "data/Funcionarios.txt"
#define FICH_CLIENTES      "data/Clientes.txt"
#define FICH_HISTORICO     "historico.csv"
#define FICH_RESULTADO     "resultado.txt"

/* ---- Numeros aleatorios ---- */
int Aleatorio(int min, int max);            /* inteiro ao acaso em [min, max] */

/* ---- Leitura validada do teclado ---- */
void  LimparBuffer(void);
int   LerInteiro(char *txt);                /* devolve o numero, ou -1 se invalido */
int   LerOpcao(char *txt, int min, int max);/* insiste ate o valor estar no intervalo */
float LerFloat(char *txt);                  /* devolve o numero, ou -1 se invalido */
void  LerString(char *txt, char *destino, int tamanho);
bool  Confirmar(char *txt);                 /* mostra "txt (S/N)" e devolve true se 'S' */

/* ---- Strings ---- */
char ToMaiscula(char x);
void CopiarNome(char *destino, char *origem); /* copia segura para um buffer [MAX_NOME] */

/* ---- Cores ANSI (paleta 256, tons suaves) ----
   Usamos sequencias ANSI; em Windows e' preciso activar Virtual Terminal e
   UTF-8 no arranque (ver AtivarCoresTerminal). */
#define COR_OK    "\x1b[38;5;108m"   /* verde-salva (fila <= 3) */
#define COR_WARN  "\x1b[38;5;179m"   /* mostarda    (fila <= 6) */
#define COR_ERR   "\x1b[38;5;174m"   /* vermelho-coral (fila > 6) */
#define COR_HDR   "\x1b[38;5;110m"   /* azul-acinzentado (cabecalhos) */
#define COR_DIM   "\x1b[38;5;244m"   /* cinza claro (separadores / FECHADA) */
#define COR_RESET "\x1b[0m"

void AtivarCoresTerminal(void);     /* habilita ANSI + UTF-8 no Windows */
void ImprimirBarraFila(int n);      /* "Fila: N pessoas [●●●·······]" colorida */

/* ---- Pausa (para a simulacao poder ser observada) ---- */
void wait_segundos(int seconds);

/* ---- Teclado em tempo real (usado pela simulacao automatica) ---- */
void DormirMs(int ms);        /* pausa de ms milissegundos sem gastar CPU */
int  TeclaPressionada(void);  /* 1 se houver uma tecla a espera (nao bloqueia) */
void DescartarTecla(void);    /* consome a(s) tecla(s) que estejam no buffer */
int  EsperarOuTecla(int ms);  /* espera ms; devolve 1 se uma tecla foi premida antes */

/* ---- Historico das acoes do utilizador, gravado em CSV (requisito 3) ---- */
void IniciarHistorico(void);                      /* marca o inicio de uma sessao */
/* Acrescenta uma linha ao historico.csv: "data;accao;detalhe".
   'detalhe' pode ser NULL ou "" quando a accao nao tem parametros. */
void RegistarHistorico(char *accao, char *detalhe);

#endif // UTEIS_H_INCLUDED
