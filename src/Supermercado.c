/**
 * @file Supermercado.c
 * @brief Ciclo de vida do Supermercado e leitura dos ficheiros de entrada.
 *
 * A simulacao em si esta em Simulacao.c, as acoes do gerente em Acoes.c
 * e os relatorios em Relatorios.c.
 */

#include <string.h>
#include "Supermercado.h"

/* ---- limites internos do carregamento ---- */
#define POOL_CLIENTES_MIN  40   /* pool minimo se o ficheiro Clientes.txt falhar */
#define MAX_CARRINHO       30   /* artigos maximos dos clientes gerados ao acaso */

/**
 * @brief Garante que cada produto carregado respeita os limites da config.
 *
 * Preco em ]0, MAX_PRECO] e tempos em [2, TEMPO_ATENDIMENTO_PRODUTO].
 */
static void ClamparProdutos(Supermercado *S)
{
    int i;
    float maxPreco = (float) S->MAX_PRECO;
    float maxTempo = (float) S->TEMPO_ATENDIMENTO_PRODUTO;
    for (i = 0; i < S->produtos.total; i++) {
        Produto *p = &S->produtos.v[i];
        if (p->preco <= 0)              p->preco = 1;
        if (p->preco > maxPreco)        p->preco = maxPreco;
        if (p->tempoComprar < 2)        p->tempoComprar = 2;
        if (p->tempoComprar > maxTempo) p->tempoComprar = maxTempo;
        if (p->tempoPagar < 2)          p->tempoPagar = 2;
        if (p->tempoPagar > maxTempo)   p->tempoPagar = maxTempo;
    }
}

/** @brief Aplica um par "CHAVE valor" da configuracao na estrutura. */
static void AplicarConfig(Supermercado *S, char *chave, int valor)
{
    if      (strcmp(chave, "MAX_ESPERA") == 0)                S->MAX_ESPERA = valor;
    else if (strcmp(chave, "N_CAIXAS") == 0)                  S->N_CAIXAS = valor;
    else if (strcmp(chave, "TEMPO_ATENDIMENTO_PRODUTO") == 0) S->TEMPO_ATENDIMENTO_PRODUTO = valor;
    else if (strcmp(chave, "MAX_PRECO") == 0)                 S->MAX_PRECO = valor;
    else if (strcmp(chave, "MAX_FILA") == 0)                  S->MAX_FILA = valor;
    else if (strcmp(chave, "MIN_FILA") == 0)                  S->MIN_FILA = valor;
    else if (strcmp(chave, "LIMITE_FILA_CAIXA") == 0)         S->LIMITE_FILA_CAIXA = valor;
    else if (strcmp(chave, "VELOCIDADE_RELOGIO") == 0)        S->VELOCIDADE_RELOGIO = valor;
    else if (strcmp(chave, "HORA_ABERTURA") == 0)             S->HORA_ABERTURA = valor;
    else if (strcmp(chave, "HORA_FECHO") == 0)                S->HORA_FECHO = valor;
    else if (strcmp(chave, "CADENCIA_ENTRADA") == 0)          S->CADENCIA_ENTRADA = valor;
    else if (strcmp(chave, "INTERVALO_AUTO_MS") == 0)         S->INTERVALO_AUTO_MS = valor;
    else if (strcmp(chave, "TICKS_POR_FRAME") == 0)           S->TICKS_POR_FRAME = valor;
}

/* =====================================================================
   API publica - ciclo de vida e I/O
   ===================================================================== */

/**
 * @brief Aloca e inicializa um Supermercado com valores por omissao.
 * @param nome Nome do supermercado (copiado para memoria propria).
 * @return Apontador para a nova instancia.
 */
Supermercado *CriarSupermercado(char *nome)
{
    Supermercado *S = (Supermercado *) malloc(sizeof(Supermercado));
    S->nome = (char *) malloc(strlen(nome) + 1);
    strcpy(S->nome, nome);
    CriarListaProdutos(&S->produtos);
    CriarListaClientes(&S->clientes);
    CriarListaFuncionarios(&S->funcionarios);
    CriarFila(&S->emCompras);
    CriarHashing(&S->caixas);
    /* valores por omissao (caso falte o ficheiro de configuracao) */
    S->MAX_ESPERA = 120; S->N_CAIXAS = 6; S->TEMPO_ATENDIMENTO_PRODUTO = 6;
    S->MAX_PRECO = 40;   S->MAX_FILA = 7; S->MIN_FILA = 3;
    S->LIMITE_FILA_CAIXA = 10;
    S->VELOCIDADE_RELOGIO = 10;
    S->HORA_ABERTURA = 8;  S->HORA_FECHO = 20;
    S->CADENCIA_ENTRADA = 70;
    S->INTERVALO_AUTO_MS = 2000;
    S->TICKS_POR_FRAME = 3;
    S->relogio = CriarRelogio(S->VELOCIDADE_RELOGIO);
    S->aceitarEntradas = true;
    S->verboso = true;
    S->totalAtendidos = 0; S->totalProdutosVendidos = 0; S->somaTemposEspera = 0;
    S->produtosOferecidos = 0; S->custoOferecido = 0; S->caixasAbertasMax = 0;
    S->totalDinheiro = 0;
    S->entradasDesdeUpdate = 0;
    S->saidasDesdeUpdate = 0;
    S->nomesEntradas[0] = '\0';
    S->ofertasDesdeUpdate = 0;
    S->nomesOfertas[0] = '\0';
    S->maxFilaObservada = 0;
    S->dentroLojaPico = 0;
    S->horaPico = 0;
    {
        int k;
        for (k = 0; k < 24; k++) {
            S->entradasPorHora[k] = 0;
            S->saidasPorHora[k] = 0;
        }
    }
    return S;
}

/**
 * @brief Le o ficheiro de configuracao.
 *
 * Pares "CHAVE valor", um por linha. Linhas iniciadas por '#' sao
 * tratadas como comentarios e ignoradas.
 * @return 1 se conseguiu abrir o ficheiro, 0 caso contrario.
 */
int CarregarConfiguracao(Supermercado *S, char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");
    char linha[128], chave[64];
    int valor;
    if (f == NULL) return 0;
    while (fgets(linha, sizeof(linha), f) != NULL) {
        if (linha[0] == '#' || linha[0] == '\n') continue;
        if (sscanf(linha, "%63s %d", chave, &valor) == 2)
            AplicarConfig(S, chave, valor);
    }
    fclose(f);
    return 1;
}

/**
 * @brief Cria a Caixa1 (unica caixa inicial, activa, com primeiro operador livre).
 *
 * O sistema arranca sempre com apenas uma caixa aberta; as restantes
 * (ate N_CAIXAS) sao abertas dinamicamente por GerirCaixas / o gerente
 * quando a media de fila ultrapassar MAX_FILA.
 */
static void CriarCaixaInicial(Supermercado *S)
{
    Caixa *cx = CriarCaixa("Caixa1", true);
    Funcionario *op = ObterFuncionarioLivre(&S->funcionarios);
    if (op != NULL) { cx->operador = op; op->ocupado = true; }
    InserirCaixa(&S->caixas, cx);
}

/** @brief Le todos os ficheiros, prepara o relogio e cria a Caixa1 inicial. */
int InicializarSupermercado(Supermercado *S)
{
    CarregarConfiguracao(S, FICH_CONFIG);
    if (S->LIMITE_FILA_CAIXA < 1) S->LIMITE_FILA_CAIXA = 10;
    /* sincroniza a velocidade do relogio com o valor (talvez) lido da config */
    S->relogio->velocidade = S->VELOCIDADE_RELOGIO;
    CarregarProdutos(&S->produtos, FICH_PRODUTOS);
    ClamparProdutos(S);                                  /* impoe limites da config */
    CarregarFuncionarios(&S->funcionarios, FICH_FUNCIONARIOS);
    CarregarClientes(&S->clientes, FICH_CLIENTES);
    CriarCaixaInicial(S);                                /* arranca so com Caixa1 */
    if (S->clientes.total < POOL_CLIENTES_MIN)
        GerarClientesAleatorios(&S->clientes, POOL_CLIENTES_MIN - S->clientes.total, MAX_CARRINHO);
    /* poe o relogio a hora de abertura (ex.: 08:00) */
    S->relogio->tempoAtual = S->HORA_ABERTURA * 3600;
    return 1;
}

/**
 * @brief Liberta toda a memoria dinamica.
 *
 * Os arrays-mestre ficam dentro da struct principal, por isso sao
 * libertados com ela.
 */
void DestruirSupermercado(Supermercado *S)
{
    DestruirHashing(&S->caixas);          /* caixas, filas e listas de atendidos */
    DestruirFila(&S->emCompras);          /* nos da lista de compras */
    DestruirListaClientes(&S->clientes);  /* tabela de indices de nomes */
    DestruirListaProdutos(&S->produtos);  /* tabela de indices de nomes */
    DestruirRelogio(S->relogio);
    free(S->nome);
    free(S);
}
