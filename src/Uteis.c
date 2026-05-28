/* Uteis.c - implementacao das funcoes auxiliares declaradas em Uteis.h */

#include <string.h>
#include <time.h>

/* Cabecalhos para deteccao de teclas em tempo real (dependem do SO). */
#ifdef _WIN32
  #include <conio.h>     /* _kbhit, _getch */
  #include <windows.h>   /* Sleep */
#else
  #include <unistd.h>    /* usleep */
  #include <termios.h>   /* modo nao-canonico do terminal */
  #include <fcntl.h>     /* leitura nao-bloqueante */
#endif

#include "Uteis.h"

/* Devolve um inteiro aleatorio no intervalo [min, max]. */
int Aleatorio(int min, int max)
{
    if (max < min) return min;
    return min + rand() % (max - min + 1);
}

/* Esvazia o resto da linha que ficou no buffer de entrada. */
void LimparBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* descarta */ }
}

/* Le uma linha e tenta interpreta-la como um inteiro.
   Devolve -1 se o utilizador nao escreveu um numero valido. */
int LerInteiro(char *txt)
{
    char linha[64];
    int valor;
    printf("%s ", txt);
    fflush(stdout);
    if (fgets(linha, sizeof(linha), stdin) == NULL) return -1;
    if (sscanf(linha, "%d", &valor) != 1) return -1;
    return valor;
}

/* Le um inteiro e so o aceita dentro de [min, max]; senao volta a pedir. */
int LerOpcao(char *txt, int min, int max)
{
    int op;
    do {
        op = LerInteiro(txt);
        if (op < min || op > max)
            printf("  Opcao invalida! Escolha entre %d e %d.\n", min, max);
    } while (op < min || op > max);
    return op;
}

/* Le um numero decimal (ex.: preco). Devolve -1 se invalido. */
float LerFloat(char *txt)
{
    char linha[64];
    float valor;
    printf("%s ", txt);
    fflush(stdout);
    if (fgets(linha, sizeof(linha), stdin) == NULL) return -1;
    if (sscanf(linha, "%f", &valor) != 1) return -1;
    return valor;
}

/* Le uma linha de texto para 'destino', cortando o '\n' final. */
void LerString(char *txt, char *destino, int tamanho)
{
    printf("%s ", txt);
    fflush(stdout);
    if (fgets(destino, tamanho, stdin) == NULL) {
        destino[0] = '\0';
        return;
    }
    destino[strcspn(destino, "\r\n")] = '\0';
}

/* Converte um caracter para maiuscula (apenas a-z). */
char ToMaiscula(char x)
{
    if (x >= 'a' && x <= 'z') return 'A' + (x - 'a');
    return x;
}

/* Copia um nome de forma segura para um buffer de tamanho MAX_NOME. */
void CopiarNome(char *destino, char *origem)
{
    snprintf(destino, MAX_NOME, "%s", origem);
}

/* Espera (aproximadamente) 'seconds' segundos, para a simulacao poder
   ser observada quando se corre passo a passo. */
void wait_segundos(int seconds)
{
    clock_t fim = clock() + (clock_t)(seconds * CLOCKS_PER_SEC);
    while (clock() < fim) { /* espera ativa */ }
}

/* Escreve um separador de sessao no inicio do historico. */
void IniciarHistorico(void)
{
    FILE *f = fopen(FICH_HISTORICO, "a");
    time_t agora = time(NULL);
    char data[32];
    if (f == NULL) return;
    strftime(data, sizeof(data), "%Y-%m-%d %H:%M:%S", localtime(&agora));
    fprintf(f, "=== Nova sessao;%s\n", data);
    fclose(f);
}

/* Acrescenta uma acao do utilizador ao ficheiro historico.csv (data;acao). */
void RegistarHistorico(char *accao)
{
    FILE *f = fopen(FICH_HISTORICO, "a");
    time_t agora = time(NULL);
    char data[32];
    if (f == NULL) return;
    strftime(data, sizeof(data), "%Y-%m-%d %H:%M:%S", localtime(&agora));
    fprintf(f, "%s;%s\n", data, accao);
    fclose(f);
}

/* =====================================================================
   Deteccao de teclas em tempo real, usada pela simulacao automatica.
   Em Windows usamos a biblioteca conio (_kbhit/_getch); noutros sistemas
   colocamos o terminal em modo nao-canonico e lemos sem bloquear.
   ===================================================================== */

#ifdef _WIN32

void DormirMs(int ms) { Sleep(ms); }

int TeclaPressionada(void) { return _kbhit(); }

void DescartarTecla(void) { while (_kbhit()) _getch(); }

#else

void DormirMs(int ms) { usleep(ms * 1000); }

/* Verifica, sem bloquear, se ha uma tecla a espera (consome-a se existir). */
int TeclaPressionada(void)
{
    struct termios antigo, novo;
    int ch, flags;
    tcgetattr(STDIN_FILENO, &antigo);
    novo = antigo;
    novo.c_lflag &= ~(ICANON | ECHO);          /* modo cru, sem eco */
    tcsetattr(STDIN_FILENO, TCSANOW, &novo);
    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &antigo);  /* repoe o terminal */
    fcntl(STDIN_FILENO, F_SETFL, flags);
    return (ch != EOF) ? 1 : 0;
}

void DescartarTecla(void) { /* a tecla ja foi consumida em TeclaPressionada */ }

#endif

/* Espera 'ms' milissegundos, mas verifica o teclado a cada 100 ms.
   Devolve 1 se uma tecla foi premida durante a espera. */
int EsperarOuTecla(int ms)
{
    int passado = 0;
    while (passado < ms) {
        if (TeclaPressionada()) return 1;
        DormirMs(100);
        passado += 100;
    }
    return TeclaPressionada();
}
