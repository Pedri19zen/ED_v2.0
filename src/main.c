/* main.c - ponto de entrada do programa.
   Carrega os dados, mostra os menus e, no fim, grava e liberta tudo.
   A simulacao SO arranca quando o utilizador escolhe "Iniciar simulacao". */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "Supermercado.h"

#define INTERVALO_AUTO_MS 2000   /* a simulacao mostra o estado a cada 2 segundos */

/* Avanca a simulacao 'segundos' segundos de uma so vez (sem imprimir os
   eventos, so o estado final). Usado pela opcao "saltar X segundos". */
static void SaltarSegundos(Supermercado *S, int segundos)
{
    int alvo = GetTempo(S->relogio) + segundos;
    S->verboso = false;                 /* avanco rapido e silencioso */
    while (GetTempo(S->relogio) < alvo)
        ExecutarPasso(S);
    S->verboso = true;
    VerEstadoAtual(S);
}

/* Menu mostrado quando o utilizador interrompe a simulacao com uma tecla.
   Devolve 1 se a simulacao deve terminar, 0 se deve continuar automatica. */
static int MenuPausaSimulacao(Supermercado *S)
{
    int op, x;
    do {
        printf("\n--- Simulacao em pausa ---\n");
        printf("1 - Continuar (automatico)\n");
        printf("2 - Saltar X segundos\n");
        printf("3 - Avancar 1 passo\n");
        printf("4 - Ver estado atual\n");
        printf("0 - Terminar simulacao (voltar ao menu)\n");
        op = LerOpcao("Opcao:", 0, 4);
        switch (op) {
            case 1: return 0;                       /* retomar automatico */
            case 2:
                x = LerInteiro("Saltar quantos segundos?");
                if (x > 0) SaltarSegundos(S, x);
                else printf("Valor invalido.\n");
                break;
            case 3: ExecutarPasso(S); VerEstadoAtual(S); break;
            case 4: VerEstadoAtual(S); break;
            case 0: return 1;                       /* terminar */
        }
    } while (1);
}

/* "Iniciar simulacao": corre sozinha, mostrando o estado a cada 4 segundos,
   ate o utilizador premir uma tecla (que abre o menu de pausa). */
static void MenuSimulacao(Supermercado *S)
{
    int terminar = 0;
    printf("\n=== Simulacao a correr ===\n");
    printf("(A simulacao avanca sozinha. Prima uma tecla para o menu de controlo.)\n");
    while (!terminar) {
        ExecutarPasso(S);
        VerEstadoAtual(S);
        fflush(stdout);                             /* mostra ja este "frame" */
        if (EsperarOuTecla(INTERVALO_AUTO_MS)) {    /* tecla premida antes dos 4 s */
            DescartarTecla();
            terminar = MenuPausaSimulacao(S);
        }
    }
    RegistarHistorico("Usou a simulacao");
}

/* Submenu com as acoes do gerente sobre caixas e clientes. */
static void MenuGerente(Supermercado *S)
{
    int op;
    char nome[MAX_NOME], caixa[MAX_NOME];
    do {
        printf("\n=== Acoes do Gerente ===\n");
        printf("1 - Abrir nova caixa\n");
        printf("2 - Fechar uma caixa agora (redistribui clientes)\n");
        printf("3 - Mover cliente para outra caixa\n");
        printf("0 - Voltar\n");
        op = LerOpcao("Opcao:", 0, 3);
        switch (op) {
            case 1:
                if (AbrirNovaCaixa(S)) RegistarHistorico("Abriu uma caixa");
                else printf("Nao foi possivel (sem operadores livres ou limite atingido).\n");
                break;
            case 2:
                LerString("Nome da caixa a fechar:", caixa, MAX_NOME);
                if (FecharCaixaImediato(S, caixa)) RegistarHistorico("Fechou uma caixa");
                break;
            case 3:
                LerString("Nome do cliente:", nome, MAX_NOME);
                LerString("Caixa de destino:", caixa, MAX_NOME);
                if (MoverClienteEntreCaixas(S, nome, caixa)) RegistarHistorico("Moveu um cliente");
                break;
        }
    } while (op != 0);
}

/* Menu principal do programa. */
static void MenuPrincipal(Supermercado *S)
{
    int op;
    char nome[MAX_NOME];
    do {
        printf("\n############ %s ############\n", S->nome);
        printf(" 1 - Iniciar simulacao\n");
        printf(" 2 - Gerir produtos\n");
        printf(" 3 - Gerir clientes\n");
        printf(" 4 - Gerir funcionarios\n");
        printf(" 5 - Ver dados atuais\n");
        printf(" 6 - Pesquisar pessoa\n");
        printf(" 7 - Acoes do gerente (abrir/fechar/mover caixa)\n");
        printf(" 8 - Medidas de desempenho\n");
        printf(" 9 - Memoria utilizada/desperdicada\n");
        printf("10 - Gravar dados (relatorio)\n");
        printf(" 0 - Sair\n");
        op = LerOpcao("Opcao:", 0, 10);
        switch (op) {
            case 1: RegistarHistorico("Abriu menu de simulacao"); MenuSimulacao(S); break;
            case 2: RegistarHistorico("Abriu gestao de produtos"); MenuProdutos(&S->produtos); break;
            case 3: RegistarHistorico("Abriu gestao de clientes"); MenuClientes(&S->clientes); break;
            case 4: RegistarHistorico("Abriu gestao de funcionarios"); MenuFuncionarios(&S->funcionarios); break;
            case 5: VerEstadoAtual(S); break;
            case 6:
                LerString("Nome da pessoa:", nome, MAX_NOME);
                PesquisarPessoa(S, nome);
                RegistarHistorico("Pesquisou uma pessoa");
                break;
            case 7: MenuGerente(S); break;
            case 8: MedidasDesempenho(S); RegistarHistorico("Viu medidas de desempenho"); break;
            case 9: MostrarMemoria(S); break;
            case 10:
                if (GravarDados(S, FICH_RESULTADO)) {
                    printf("Relatorio gravado em %s\n", FICH_RESULTADO);
                    RegistarHistorico("Gravou o relatorio");
                } else printf("Falha ao gravar.\n");
                break;
        }
    } while (op != 0);
}

int main(void)
{
    Supermercado *S;

    /* Semente do gerador de aleatorios. Para depurar de forma repetivel,
       basta substituir time(NULL) por um valor fixo, por exemplo srand(1). */
    srand((unsigned) time(NULL));

    IniciarHistorico();
    printf("Projeto ED 25/26 - Gestao de Caixas de um Supermercado\n");

    S = CriarSupermercado("Compra Aqui Lda.");
    InicializarSupermercado(S);
    printf("Dados carregados: %d produtos, %d clientes, %d funcionarios, %d caixas.\n",
           S->produtos.total, S->clientes.total, S->funcionarios.total, S->caixas.totalCaixas);

    MenuPrincipal(S);

    GravarDados(S, FICH_RESULTADO);   /* grava o relatorio final ao sair (req. 2) */
    RegistarHistorico("Saiu do programa");
    DestruirSupermercado(S);

    printf("Ate breve!\n");
    return 0;
}
