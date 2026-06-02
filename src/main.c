/* main.c - ponto de entrada do programa.
   Carrega os dados, mostra os menus e, no fim, grava e liberta tudo.
   A simulacao SO arranca quando o utilizador escolhe "Iniciar simulacao". */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "Supermercado.h"

#define INTERVALO_AUTO_MS 2000   /* a simulacao mostra o estado a cada 2 segundos */
#define TICKS_POR_FRAME   3      /* passos de simulacao avancados em cada frame */

/* Avanca a simulacao 'segundos' segundos de uma so vez (sem imprimir os
   eventos, so o estado final). Usado pela opcao "saltar X segundos". */
static void SaltarSegundos(Supermercado *S, int segundos)
{
    int alvo = GetTempo(S->relogio) + segundos;
    bool anterior = S->verboso;
    S->verboso = false;                 /* avanco rapido e silencioso */
    while (GetTempo(S->relogio) < alvo)
        ExecutarPasso(S);
    S->verboso = anterior;
    VerEstadoAtual(S);
}

/* Menu mostrado quando o utilizador interrompe a simulacao com uma tecla.
   Enquanto este menu estiver aberto a simulacao fica PARADA (o tempo nao
   avanca). Devolve 1 se a simulacao deve terminar, 0 se deve retomar. */
static int MenuPausaSimulacao(Supermercado *S)
{
    int op, x;
    do {
        printf("\n--- Simulacao PAUSADA (tempo congelado) ---\n");
        printf("1 - Retomar simulacao\n");
        printf("2 - Avancar 1 passo\n");
        printf("3 - Saltar X segundos\n");
        printf("4 - Ver estado atual\n");
        printf("0 - Parar e voltar ao menu principal\n");
        op = LerOpcao("Opcao:", 0, 4);
        switch (op) {
            case 1: return 0;                       /* retomar automatico */
            case 2: ExecutarPasso(S); VerEstadoAtual(S); break;
            case 3:
                x = LerInteiro("Saltar quantos segundos?");
                if (x > 0) SaltarSegundos(S, x);
                else printf("Valor invalido.\n");
                break;
            case 4: VerEstadoAtual(S); break;
            case 0: return 1;                       /* terminar */
        }
    } while (1);
}

/* "Iniciar simulacao": corre sozinha, mostrando o estado a cada INTERVALO_AUTO_MS,
   ate o utilizador premir uma tecla (que abre o menu de pausa). */
static void MenuSimulacao(Supermercado *S)
{
    int terminar = 0, k;
    bool verbosoAnterior = S->verboso;
    S->verboso = false;                  /* so o "frame" boxed aparece durante o auto */
    printf("\n=== Simulacao a correr ===\n");
    printf("(Loja aberta entre 08:00 e 20:00. Prima uma tecla para PAUSAR.)\n");
    while (!terminar) {
        for (k = 0; k < TICKS_POR_FRAME; k++) ExecutarPasso(S);
        VerEstadoAtual(S);
        fflush(stdout);                  /* mostra ja este "frame" */
        if (!LojaAberta(S) && SimulacaoTerminada(S)) {
            printf("\n*** Loja fechou e ja nao ha clientes. Fim do dia. ***\n");
            break;
        }
        if (EsperarOuTecla(INTERVALO_AUTO_MS)) {
            DescartarTecla();
            terminar = MenuPausaSimulacao(S);
        }
    }
    S->verboso = verbosoAnterior;
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
                if (!Confirmar("Tem a certeza que quer fechar esta caixa?")) {
                    printf("Operacao cancelada.\n"); break;
                }
                if (FecharCaixaImediato(S, caixa)) RegistarHistorico("Fechou uma caixa");
                break;
            case 3:
                LerString("Nome do cliente:", nome, MAX_NOME);
                LerString("Caixa de destino:", caixa, MAX_NOME);
                if (!Confirmar("Confirma a mudanca de caixa?")) {
                    printf("Operacao cancelada.\n"); break;
                }
                if (MoverClienteEntreCaixas(S, nome, caixa)) RegistarHistorico("Moveu um cliente");
                break;
        }
    } while (op != 0);
}

/* Submenu: arrancar a simulacao ou fazer acoes do gerente. */
static void MenuSimulacaoTopo(Supermercado *S)
{
    int op;
    do {
        printf("\n--- Simulacao ---\n");
        printf("1 - Iniciar/retomar simulacao\n");
        printf("2 - Acoes do gerente (abrir/fechar/mover caixa)\n");
        printf("0 - Voltar\n");
        op = LerOpcao("Opcao:", 0, 2);
        switch (op) {
            case 1: RegistarHistorico("Abriu menu de simulacao"); MenuSimulacao(S); break;
            case 2: MenuGerente(S); break;
        }
    } while (op != 0);
}

/* Submenu: gestao dos dados-mestre (produtos, clientes, funcionarios). */
static void MenuGestao(Supermercado *S)
{
    int op;
    do {
        printf("\n--- Gestao de Dados ---\n");
        printf("1 - Gerir produtos\n");
        printf("2 - Gerir clientes\n");
        printf("3 - Gerir funcionarios\n");
        printf("0 - Voltar\n");
        op = LerOpcao("Opcao:", 0, 3);
        switch (op) {
            case 1: RegistarHistorico("Abriu gestao de produtos");
                    MenuProdutos(&S->produtos); break;
            case 2: RegistarHistorico("Abriu gestao de clientes");
                    MenuClientes(&S->clientes); break;
            case 3: RegistarHistorico("Abriu gestao de funcionarios");
                    MenuFuncionarios(&S->funcionarios); break;
        }
    } while (op != 0);
}

/* Submenu: consultas e relatorios. */
static void MenuConsultas(Supermercado *S)
{
    int op;
    char nome[MAX_NOME];
    do {
        printf("\n--- Consultas e Relatorios ---\n");
        printf("1 - Ver estado atual\n");
        printf("2 - Pesquisar pessoa\n");
        printf("3 - Medidas de desempenho\n");
        printf("4 - Memoria utilizada/desperdicada\n");
        printf("0 - Voltar\n");
        op = LerOpcao("Opcao:", 0, 4);
        switch (op) {
            case 1: VerEstadoAtual(S); break;
            case 2:
                LerString("Nome da pessoa:", nome, MAX_NOME);
                PesquisarPessoa(S, nome);
                RegistarHistorico("Pesquisou uma pessoa");
                break;
            case 3: MedidasDesempenho(S);
                    RegistarHistorico("Viu medidas de desempenho"); break;
            case 4: MostrarMemoria(S); break;
        }
    } while (op != 0);
}

/* Menu principal do programa (5 entradas). */
static void MenuPrincipal(Supermercado *S)
{
    int op;
    do {
        printf("\n############ %s ############\n", S->nome);
        printf("1 - Simulacao\n");
        printf("2 - Gestao de dados\n");
        printf("3 - Consultas e relatorios\n");
        printf("4 - Gravar relatorio\n");
        printf("0 - Sair\n");
        op = LerOpcao("Opcao:", 0, 4);
        switch (op) {
            case 1: MenuSimulacaoTopo(S); break;
            case 2: MenuGestao(S); break;
            case 3: MenuConsultas(S); break;
            case 4:
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
