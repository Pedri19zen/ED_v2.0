/**
 * @file Acoes.c
 * @brief Acoes do gerente sobre caixas e clientes (requisitos 4, 5, 7, 8).
 *
 * Inclui tambem helpers de UX usados pelos menus.
 */

#include <string.h>
#include "Supermercado.h"

/**
 * @brief Requisito 5: abre uma caixa.
 *
 * Reutiliza uma caixa fechada se existir; se nao houver e ainda nao se
 * atingiu N_CAIXAS, cria uma nova. Precisa de um operador livre.
 * Os clientes ja em fila nas outras caixas NAO sao movidos -- a nova
 * caixa comeca vazia e recebe naturalmente os clientes que acabam as
 * compras (via EscolherMelhorCaixa) e ainda nao chegaram ao LIMITE_FILA_CAIXA
 * das antigas.
 * @return 1 se conseguiu, 0 caso contrario.
 */
int AbrirNovaCaixa(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS], *fechada = NULL;
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i;
    Funcionario *op;
    char nome[MAX_NOME];
    for (i = 0; i < n; i++)
        if (!vec[i]->ativa) { fechada = vec[i]; break; }
    op = ObterFuncionarioLivre(&S->funcionarios);
    if (op == NULL) return 0;                       /* sem operadores disponiveis */
    if (fechada != NULL) {                           /* reabre uma caixa fechada */
        fechada->ativa = true;
        fechada->aFechar = false;
        fechada->operador = op; op->ocupado = true;
        if (S->verboso) printf("  [Caixa aberta] %s\n", fechada->nome);
        return 1;
    }
    if (S->caixas.totalCaixas < S->N_CAIXAS) {       /* cria uma caixa nova */
        Caixa *nova;
        snprintf(nome, MAX_NOME, "Caixa%d", S->caixas.totalCaixas + 1);
        nova = CriarCaixa(nome, true);
        nova->operador = op; op->ocupado = true;
        InserirCaixa(&S->caixas, nova);
        if (S->verboso) printf("  [Caixa aberta] %s (nova)\n", nome);
        return 1;
    }
    return 0;
}

/**
 * @brief Escolhe uma caixa de destino com espaco; se necessario tenta abrir uma.
 *
 * Usado apenas no fecho imediato: se todas as restantes caixas estiverem
 * cheias, tenta criar/reabrir outra antes de desistir.
 */
static Caixa *EscolherDestinoOuAbrir(Supermercado *S, Caixa *excluir)
{
    Caixa *destino = EscolherMelhorCaixa(S, excluir);
    if (destino == NULL && AbrirNovaCaixa(S))
        destino = EscolherMelhorCaixa(S, excluir);
    return destino;
}

/**
 * @brief Requisito 7: fecha uma caixa de imediato.
 *
 * Distribui os clientes em fila pelas restantes caixas abertas, respeitando
 * LIMITE_FILA_CAIXA. Se nao houver destino com espaco, a caixa nao fecha.
 */
int FecharCaixaImediato(Supermercado *S, char *nomeCaixa)
{
    Caixa *cx = PesquisarCaixa(&S->caixas, nomeCaixa);
    Caixa *destino;
    Cliente *c;
    if (cx == NULL)  { printf("Caixa nao encontrada.\n"); return 0; }
    if (!cx->ativa)  { printf("Essa caixa ja esta fechada.\n"); return 0; }
    if (ContarCaixasAbertas(S) <= 1) {
        /* nao deixar a loja sem caixas: tenta abrir outra primeiro */
        if (!AbrirNovaCaixa(S)) {
            printf("Nao ha outra caixa para receber os clientes.\n");
            return 0;
        }
    }
    /* move o cliente que estava a ser atendido */
    if (cx->aAtender != NULL) {
        destino = EscolherDestinoOuAbrir(S, cx);
        if (destino != NULL) {
            c = cx->aAtender;
            cx->aAtender = NULL;
            EnfileirarCliente(&destino->fila, c);
            CopiarNome(c->caixaAtual, destino->nome);
        } else {
            printf("Nao ha caixa com espaco para receber os clientes.\n");
            return 0;
        }
    }
    /* move os que estavam em espera */
    while (!FilaVazia(&cx->fila)) {
        c = DesenfileirarCliente(&cx->fila);
        destino = EscolherDestinoOuAbrir(S, cx);
        if (destino == NULL) {
            EnfileirarCliente(&cx->fila, c);
            printf("Nao ha caixa com espaco para receber todos os clientes.\n");
            return 0;
        }
        EnfileirarCliente(&destino->fila, c);
        CopiarNome(c->caixaAtual, destino->nome);
    }
    cx->ativa = false;
    cx->aFechar = false;
    if (cx->operador != NULL) { cx->operador->ocupado = false; cx->operador = NULL; }
    printf("Caixa %s fechada e clientes redistribuidos.\n", cx->nome);
    return 1;
}

/** @brief Requisito 4: muda um cliente que esta numa fila para outra caixa. */
int MoverClienteEntreCaixas(Supermercado *S, char *nomeCliente, char *nomeCaixa)
{
    int idx = PesquisarCliente(&S->clientes, nomeCliente);
    Cliente *c;
    Caixa *destino, *origem;
    if (idx < 0) { printf("Cliente nao encontrado.\n"); return 0; }
    c = &S->clientes.v[idx];
    if (!c->dentroLoja || !c->naFila) { printf("O cliente nao esta numa fila.\n"); return 0; }
    destino = PesquisarCaixa(&S->caixas, nomeCaixa);
    if (destino == NULL || !destino->ativa) { printf("Caixa de destino invalida.\n"); return 0; }
    if (strcmp(c->caixaAtual, nomeCaixa) == 0) { printf("Ja esta nessa caixa.\n"); return 0; }
    if (TamanhoFila(&destino->fila) >= S->LIMITE_FILA_CAIXA) {
        printf("Caixa de destino cheia.\n");
        return 0;
    }
    origem = PesquisarCaixa(&S->caixas, c->caixaAtual);
    if (origem != NULL) {
        if (origem->aAtender == c) origem->aAtender = NULL;
        else RemoverClienteDaFila(&origem->fila, c);
    }
    EnfileirarCliente(&destino->fila, c);
    CopiarNome(c->caixaAtual, nomeCaixa);
    printf("Cliente %s movido para %s.\n", nomeCliente, nomeCaixa);
    return 1;
}

/**
 * @brief Requisito 8: diz onde esta uma pessoa.
 *
 * Resultado possivel: fora, a fazer compras, em fila numa caixa, ou a
 * ser atendida. Aceita variacoes em maiusculas/minusculas e
 * correspondencia por prefixo.
 */
void PesquisarPessoa(Supermercado *S, char *nomeCliente)
{
    int idx = PesquisarClienteTolerante(&S->clientes, nomeCliente);
    Cliente *c;
    Caixa *cx;
    if (idx < 0) { printf("Cliente nao registado.\n"); return; }
    c = &S->clientes.v[idx];
    if (strcmp(c->nome, nomeCliente) != 0)
        printf("(correspondencia: '%s')\n", c->nome);
    if (!c->dentroLoja) { printf("%s esta fora da loja.\n", c->nome); return; }
    if (!c->naFila)     { printf("%s esta a fazer compras.\n", c->nome); return; }
    cx = PesquisarCaixa(&S->caixas, c->caixaAtual);
    if (cx != NULL && cx->aAtender == c)
        printf("%s esta a ser atendido na %s.\n", c->nome, c->caixaAtual);
    else
        printf("%s esta em espera na %s.\n", c->nome, c->caixaAtual);
}

/** @brief Requisito 11: lista os clientes ja atendidos por uma caixa especifica. */
void ListarAtendidosPorCaixa(Supermercado *S, char *nomeCaixa)
{
    Caixa *cx = PesquisarCaixa(&S->caixas, nomeCaixa);
    if (cx == NULL) { printf("Caixa nao encontrada.\n"); return; }
    ListarAtendidosCaixa(cx);
}

/**
 * @brief Resumo multi-linha de todas as caixas.
 *
 * Mostra Caixa1..CaixaN, mesmo que algumas ainda nao tenham sido criadas.
 * As caixas inexistentes aparecem como fechadas.
 */
void MostrarResumoCaixas(Supermercado *S)
{
    int i, limiteCaixas = S->N_CAIXAS;
    char nomeCaixa[MAX_NOME];
    if (limiteCaixas > MAX_CAIXAS) limiteCaixas = MAX_CAIXAS;
    printf("  %sCaixas disponiveis:%s\n", COR_HDR, COR_RESET);
    for (i = 1; i <= limiteCaixas; i++) {
        Caixa *cx;
        snprintf(nomeCaixa, sizeof(nomeCaixa), "Caixa%d", i);
        cx = PesquisarCaixa(&S->caixas, nomeCaixa);
        if (cx != NULL && cx->ativa) {
            printf("    %-8s %s[ABERTA]%s  ", nomeCaixa, COR_OK, COR_RESET);
            ImprimirBarraFila(TamanhoFila(&cx->fila));
            printf("\n");
        } else {
            printf("    %-8s %s[FECHADA]%s\n", nomeCaixa, COR_DIM, COR_RESET);
        }
    }
}

/**
 * @brief Aceita "Caixa1" ou simplesmente "1" como referencia a uma caixa.
 * @param entrada Texto introduzido pelo utilizador.
 * @param destino Buffer onde se escreve o nome canonico.
 */
void ResolverNomeCaixa(char *entrada, char *destino)
{
    char *fim;
    long n;
    n = strtol(entrada, &fim, 10);
    if (entrada[0] != '\0' && *fim == '\0' && n > 0 && n < MAX_CAIXAS)
        snprintf(destino, MAX_NOME, "Caixa%ld", n);
    else
        snprintf(destino, MAX_NOME, "%s", entrada);
}
