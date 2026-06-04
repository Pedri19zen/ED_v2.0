/**
 * @file Uteis.c
 * @brief Implementacao das funcoes auxiliares declaradas em Uteis.h.
 */

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

/** @brief Devolve um inteiro aleatorio no intervalo [min, max]. */
int Aleatorio(int min, int max)
{
    if (max < min) return min;
    return min + rand() % (max - min + 1);
}

/**
 * @brief Activa ANSI Virtual Terminal e UTF-8 no terminal Windows.
 *
 * Necessario para as cores e os caracteres da barra (●·) aparecerem
 * correctamente. Em Linux/macOS nao e' preciso fazer nada.
 */
void AtivarCoresTerminal(void)
{
#ifdef _WIN32
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD modo = 0;
    if (h != INVALID_HANDLE_VALUE && GetConsoleMode(h, &modo))
        SetConsoleMode(h, modo | 0x0004); /* ENABLE_VIRTUAL_TERMINAL_PROCESSING */
    SetConsoleOutputCP(65001);            /* UTF-8 */
#endif
}

/**
 * @brief Imprime "Fila: N pessoas [●●●·······]" colorida.
 *
 * Verde se n <= 3, amarelo se n <= 6, vermelho se n > 6. A barra tem
 * sempre 10 celulas.
 */
void ImprimirBarraFila(int n)
{
    int i, cheias;
    char *cor;
    if (n < 0) n = 0;
    cheias = (n > 10) ? 10 : n;
    if      (n <= 3) cor = COR_OK;
    else if (n <= 6) cor = COR_WARN;
    else             cor = COR_ERR;
    printf("%sFila: %2d pessoas [", cor, n);
    for (i = 0; i < cheias; i++) fputs("\xe2\x97\x8f", stdout); /* U+25CF '●' */
    for (i = cheias; i < 10; i++) fputs("\xc2\xb7", stdout);     /* U+00B7 '·' */
    printf("]%s", COR_RESET);
}

/** @brief Esvazia o resto da linha que ficou no buffer de entrada. */
void LimparBuffer(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { /* descarta */ }
}

/**
 * @brief Le uma linha e tenta interpreta-la como um inteiro.
 * @return O numero lido, ou -1 se a entrada nao for um numero valido.
 */
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

/** @brief Le um inteiro insistindo ate o valor estar em [min, max]. */
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

/** @brief Le um numero decimal (ex.: preco). @return -1 se invalido. */
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

/** @brief Le uma linha para 'destino', cortando o '\\n' final. */
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

/** @brief Pergunta "txt (S/N)" e devolve true se a resposta comecar por 'S'/'s'. */
bool Confirmar(char *txt)
{
    char linha[16];
    printf("%s (S/N) ", txt);
    fflush(stdout);
    if (fgets(linha, sizeof(linha), stdin) == NULL) return false;
    return (linha[0] == 'S' || linha[0] == 's');
}

/** @brief Converte um caracter para maiuscula (apenas a-z). */
char ToMaiscula(char x)
{
    if (x >= 'a' && x <= 'z') return 'A' + (x - 'a');
    return x;
}

/** @brief Copia um nome de forma segura para um buffer de tamanho MAX_NOME. */
void CopiarNome(char *destino, char *origem)
{
    snprintf(destino, MAX_NOME, "%s", origem);
}

/**
 * @brief Espera (aproximadamente) 'seconds' segundos em espera activa.
 *
 * Util para a simulacao poder ser observada quando se corre passo a passo.
 */
void wait_segundos(int seconds)
{
    clock_t fim = clock() + (clock_t)(seconds * CLOCKS_PER_SEC);
    while (clock() < fim) { /* espera ativa */ }
}

/** @brief Escreve um separador de sessao no inicio do historico CSV. */
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

/**
 * @brief Acrescenta uma acao do utilizador ao historico CSV.
 *
 * Formato: "data;accao;detalhe" (detalhe pode estar vazio).
 */
void RegistarHistorico(char *accao, char *detalhe)
{
    FILE *f = fopen(FICH_HISTORICO, "a");
    time_t agora = time(NULL);
    char data[32];
    if (f == NULL) return;
    strftime(data, sizeof(data), "%Y-%m-%d %H:%M:%S", localtime(&agora));
    fprintf(f, "%s;%s;%s\n", data, accao, (detalhe != NULL) ? detalhe : "");
    fclose(f);
}

/* =====================================================================
   Deteccao de teclas em tempo real, usada pela simulacao automatica.
   Em Windows usamos a biblioteca conio (_kbhit/_getch); noutros sistemas
   colocamos o terminal em modo nao-canonico e lemos sem bloquear.
   ===================================================================== */

#ifdef _WIN32

/** @brief Pausa 'ms' milissegundos (Sleep do WinAPI). */
void DormirMs(int ms) { Sleep(ms); }

/** @brief 1 se houver uma tecla a espera no buffer (nao bloqueia). */
int TeclaPressionada(void) { return _kbhit(); }

/** @brief Consome todas as teclas pendentes no buffer. */
void DescartarTecla(void) { while (_kbhit()) _getch(); }

#else

/** @brief Pausa 'ms' milissegundos (usleep). */
void DormirMs(int ms) { usleep(ms * 1000); }

/** @brief Verifica, sem bloquear, se ha uma tecla a espera (consome-a se existir). */
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

/** @brief No-op em Linux/macOS: a tecla ja foi consumida em TeclaPressionada. */
void DescartarTecla(void) { /* a tecla ja foi consumida em TeclaPressionada */ }

#endif

/**
 * @brief Espera 'ms' milissegundos verificando o teclado a cada 100 ms.
 * @return 1 se uma tecla foi premida durante a espera, 0 caso contrario.
 */
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
