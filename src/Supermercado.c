/* Supermercado.c - junta todas as estruturas e implementa a simulacao,
   as acoes do gerente e os relatorios pedidos no enunciado. */

#include <string.h>
#include "Supermercado.h"

/* ---- parametros internos da simulacao (faceis de afinar) ---- */
#define VELOCIDADE_RELOGIO        10   /* segundos de simulacao por passo */
#define CADENCIA_ENTRADA_DEFAULT  70   /* % de entrar 1 cliente em cada tentativa */
#define ENTRADAS_POR_TICK_MAX     4    /* tentativas de entrada em cada passo */
#define MAX_CARRINHO              5    /* artigos maximos de quem entra ao acaso */
#define POOL_CLIENTES_MIN         40   /* pool minimo se o ficheiro Clientes.txt falhar */
#define MAX_ITERACOES_SIM         100000 /* trava de seguranca */

/* ---- horario da loja (a loja so aceita entradas dentro deste intervalo) ---- */
#define HORA_ABERTURA             8    /* abre as 08:00 */
#define HORA_FECHO                20   /* fecha as 20:00 */

/* =====================================================================
   Funcoes auxiliares internas (static) - so usadas dentro deste ficheiro
   ===================================================================== */

/* Conta os clientes que estao dentro da loja (a comprar ou em fila). */
static int ContarDentroLoja(Supermercado *S)
{
    int i, n = 0;
    for (i = 0; i < S->clientes.total; i++)
        if (S->clientes.v[i].ativo && S->clientes.v[i].dentroLoja) n++;
    return n;
}

/* Conta as caixas que estao abertas. */
static int ContarCaixasAbertas(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS];
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i, abertas = 0;
    for (i = 0; i < n; i++)
        if (vec[i]->ativa) abertas++;
    return abertas;
}

/* Soma o numero de clientes em espera em todas as caixas. */
static int ContarClientesEmFilas(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS];
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i, total = 0;
    for (i = 0; i < n; i++)
        total += TamanhoFila(&vec[i]->fila);
    return total;
}

/* Escolhe a caixa aberta (e que nao esteja a fechar) com a fila mais curta.
   'excluir' permite ignorar uma caixa (ex.: a que esta a ser fechada). */
static Caixa *EscolherMelhorCaixa(Supermercado *S, Caixa *excluir)
{
    Caixa *vec[MAX_CAIXAS], *melhor = NULL;
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i;
    for (i = 0; i < n; i++) {
        if (!vec[i]->ativa || vec[i]->aFechar || vec[i] == excluir) continue;
        if (melhor == NULL || TamanhoFila(&vec[i]->fila) < TamanhoFila(&melhor->fila))
            melhor = vec[i];
    }
    return melhor;
}

/* Escolhe, ao acaso, um cliente registado que esteja fora da loja. */
static Cliente *EscolherClienteParaEntrar(Supermercado *S)
{
    int total = S->clientes.total, inicio, i;
    if (total == 0) return NULL;
    inicio = Aleatorio(0, total - 1);
    for (i = 0; i < total; i++) {
        Cliente *c = &S->clientes.v[(inicio + i) % total];
        if (c->ativo && !c->dentroLoja) return c;
    }
    return NULL;
}

/* True se a hora atual estiver dentro do horario de funcionamento. */
bool LojaAberta(Supermercado *S)
{
    int hora = (GetTempo(S->relogio) / 3600) % 24;
    return hora >= HORA_ABERTURA && hora < HORA_FECHO;
}

/* Passo "entrada": pode entrar mais do que um cliente por passo (durante o
   horario de funcionamento) para manter a loja com movimento. */
static void EntradaCliente(Supermercado *S)
{
    int tentativa;
    if (!S->aceitarEntradas) return;
    if (!LojaAberta(S)) return;                           /* fora de horas */
    for (tentativa = 0; tentativa < ENTRADAS_POR_TICK_MAX; tentativa++) {
        Cliente *c;
        if (Aleatorio(1, 100) > S->cadenciaEntrada) continue;  /* nao entrou */
        if (ContarDentroLoja(S) >= CAPACIDADE_LOJA) return;    /* loja cheia */
        c = EscolherClienteParaEntrar(S);
        if (c == NULL) return;                                 /* nao ha disponiveis */
        c->dentroLoja = true;
        c->naFila = false;
        c->caixaAtual[0] = '\0';
        c->numProdutos = Aleatorio(1, MAX_CARRINHO);
        PrepararCarrinho(c, &S->produtos, S->MAX_PRECO, S->TEMPO_ATENDIMENTO_PRODUTO);
        EnfileirarCliente(&S->emCompras, c);
        if (S->verboso) printf("  [Entrou] %s (%d artigos)\n", c->nome, c->numProdutos);
    }
}

/* Passo "compras -> caixa": cada cliente avanca o seu tempo de compras; quando
   chega a zero, vai para a fila da melhor caixa. */
static void AvancarCompras(Supermercado *S)
{
    Cliente *prontos[CAPACIDADE_LOJA];
    int nProntos = 0, i, vel = S->relogio->velocidade;
    NoFila *p;
    /* 1) avanca o tempo de compras de cada um; marca os que acabaram */
    for (p = S->emCompras.inicio; p != NULL && nProntos < CAPACIDADE_LOJA; p = p->prox) {
        p->cliente->tempoComprasRestante -= vel;
        if (p->cliente->tempoComprasRestante <= 0)
            prontos[nProntos++] = p->cliente;
    }
    /* 2) move-os para a fila da melhor caixa */
    for (i = 0; i < nProntos; i++) {
        Caixa *cx = EscolherMelhorCaixa(S, NULL);
        if (cx == NULL) break;            /* nenhuma caixa disponivel: fica a comprar */
        RemoverClienteDaFila(&S->emCompras, prontos[i]);
        EnfileirarCliente(&cx->fila, prontos[i]);
        prontos[i]->naFila = true;
        CopiarNome(prontos[i]->caixaAtual, cx->nome);
    }
}

/* Finaliza o atendimento do cliente que estava a ser atendido na caixa cx
   (atualiza estatisticas e poe o cliente a sair da loja). */
static void TerminarAtendimento(Supermercado *S, Caixa *cx)
{
    Cliente *c = cx->aAtender;
    /* dinheiro que o cliente paga: o valor do carrinho menos o artigo que
       lhe foi oferecido (se esteve a espera mais do que MAX_ESPERA). */
    float pago = c->valorCarrinho;
    if (c->foiOferecido) pago -= c->precoMenorProduto;
    S->somaTemposEspera += c->tempoEspera;
    S->totalAtendidos++;
    S->totalProdutosVendidos += c->numProdutos;
    S->totalDinheiro += pago;
    cx->pessoasAtendidas++;
    cx->produtosVendidos += c->numProdutos;
    cx->dinheiroFeito += pago;
    if (cx->operador != NULL) {
        cx->operador->pessoasAtendidas++;
        cx->operador->produtosVendidos += c->numProdutos;
    }
    RegistarAtendido(cx, c->nome);
    /* o cliente sai da loja */
    c->dentroLoja = false;
    c->naFila = false;
    c->caixaAtual[0] = '\0';
    cx->aAtender = NULL;
}

/* Fecha uma caixa que estava marcada para fechar e ja esvaziou. */
static void FecharCaixaSeVazia(Caixa *cx, int verboso)
{
    if (cx->aFechar && cx->aAtender == NULL && FilaVazia(&cx->fila)) {
        cx->ativa = false;
        cx->aFechar = false;
        if (cx->operador != NULL) { cx->operador->ocupado = false; cx->operador = NULL; }
        if (verboso) printf("  [Caixa fechada] %s\n", cx->nome);
    }
}

/* Passo "atendimento": cada caixa aberta atende um pouco o cliente da frente. */
static void AtenderCaixas(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS];
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i;
    int vel = S->relogio->velocidade;
    for (i = 0; i < n; i++) {
        Caixa *cx = vec[i];
        NoFila *p;
        if (!cx->ativa) continue;
        /* quem espera na fila acumula tempo de espera */
        for (p = cx->fila.inicio; p != NULL; p = p->prox)
            p->cliente->tempoEspera += vel;
        /* se ninguem esta a ser atendido, chama o proximo da fila */
        if (cx->aAtender == NULL && !FilaVazia(&cx->fila))
            cx->aAtender = DesenfileirarCliente(&cx->fila);
        /* atende o cliente atual */
        if (cx->aAtender != NULL) {
            cx->aAtender->tempoRestante -= vel;
            if (cx->aAtender->tempoRestante <= 0)
                TerminarAtendimento(S, cx);
        }
        FecharCaixaSeVazia(cx, S->verboso);
    }
}

/* Passo "garantia de qualidade": oferece um produto a quem ja esperou mais
   do que MAX_ESPERA (apenas uma vez por cliente). */
static void VerificarTemposEspera(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS];
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i;
    for (i = 0; i < n; i++) {
        NoFila *p;
        for (p = vec[i]->fila.inicio; p != NULL; p = p->prox) {
            Cliente *c = p->cliente;
            if (!c->foiOferecido && c->tempoEspera > S->MAX_ESPERA && c->numProdutos > 0) {
                c->foiOferecido = true;
                S->produtosOferecidos++;
                /* oferece-se o artigo mais barato do carrinho */
                S->custoOferecido += c->precoMenorProduto;
                if (S->verboso)
                    printf("  [Oferta] %s esperou demais; oferta de 1 produto (%.2f EUR).\n",
                           c->nome, c->precoMenorProduto);
            }
        }
    }
}

/* Passo "gerir caixas": abre ou fecha caixas conforme a media de clientes
   por fila (requisitos 5 e 6). */
static void GerirCaixas(Supermercado *S)
{
    int abertas = ContarCaixasAbertas(S);
    int emFilas = ContarClientesEmFilas(S);
    float media;
    if (abertas == 0) { AbrirNovaCaixa(S); return; }   /* garante 1 aberta */
    media = (float) emFilas / abertas;
    if (media > S->MAX_FILA) {
        AbrirNovaCaixa(S);                              /* req. 5: abrir automatico */
    } else if (media < S->MIN_FILA && abertas > 1) {
        /* req. 6: marcar para fechar a caixa com menos pessoas */
        Caixa *vec[MAX_CAIXAS], *menos = NULL;
        int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i;
        for (i = 0; i < n; i++) {
            if (!vec[i]->ativa || vec[i]->aFechar) continue;
            if (menos == NULL || TamanhoFila(&vec[i]->fila) < TamanhoFila(&menos->fila))
                menos = vec[i];
        }
        if (menos != NULL) {
            menos->aFechar = true;
            if (S->verboso) printf("  [A fechar] %s (poucas pessoas)\n", menos->nome);
        }
    }
    abertas = ContarCaixasAbertas(S);
    if (abertas > S->caixasAbertasMax) S->caixasAbertasMax = abertas;
}

/* =====================================================================
   Funcoes publicas
   ===================================================================== */

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
    S->relogio = CriarRelogio(VELOCIDADE_RELOGIO);
    /* valores por omissao (caso falte o ficheiro de configuracao) */
    S->MAX_ESPERA = 120; S->N_CAIXAS = 6; S->TEMPO_ATENDIMENTO_PRODUTO = 6;
    S->MAX_PRECO = 40;   S->MAX_FILA = 7; S->MIN_FILA = 3;
    S->cadenciaEntrada = CADENCIA_ENTRADA_DEFAULT;
    S->aceitarEntradas = true;
    S->verboso = true;
    S->totalAtendidos = 0; S->totalProdutosVendidos = 0; S->somaTemposEspera = 0;
    S->produtosOferecidos = 0; S->custoOferecido = 0; S->caixasAbertasMax = 0;
    S->totalDinheiro = 0;
    return S;
}

/* Atualiza um parametro de configuracao a partir de um par "CHAVE valor". */
static void AplicarConfig(Supermercado *S, char *chave, int valor)
{
    if      (strcmp(chave, "MAX_ESPERA") == 0)                S->MAX_ESPERA = valor;
    else if (strcmp(chave, "N_CAIXAS") == 0)                  S->N_CAIXAS = valor;
    else if (strcmp(chave, "TEMPO_ATENDIMENTO_PRODUTO") == 0) S->TEMPO_ATENDIMENTO_PRODUTO = valor;
    else if (strcmp(chave, "MAX_PRECO") == 0)                 S->MAX_PRECO = valor;
    else if (strcmp(chave, "MAX_FILA") == 0)                  S->MAX_FILA = valor;
    else if (strcmp(chave, "MIN_FILA") == 0)                  S->MIN_FILA = valor;
}

/* Le o ficheiro de configuracao (pares "CHAVE valor", um por linha). */
int CarregarConfiguracao(Supermercado *S, char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");
    char linha[128], chave[64];
    int valor;
    if (f == NULL) return 0;
    while (fgets(linha, sizeof(linha), f) != NULL)
        if (sscanf(linha, "%63s %d", chave, &valor) == 2)
            AplicarConfig(S, chave, valor);
    fclose(f);
    return 1;
}

/* Le o ficheiro de dados: numero de caixas e, para cada uma, se esta ativa,
   o numero de clientes e cada cliente (com o seu numero de produtos).
   As caixas vao para o hashing e os clientes para as respetivas filas. */
int CarregarDados(Supermercado *S, char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");
    char linha[128], nomeCaixa[MAX_NOME], nomeCli[MAX_NOME];
    int nCaixas, nClientes, ativa, nProd, i, j;
    if (f == NULL) return 0;
    if (fgets(linha, sizeof(linha), f) == NULL || sscanf(linha, "%d", &nCaixas) != 1) {
        fclose(f);
        return 0;
    }
    for (i = 0; i < nCaixas; i++) {
        Caixa *cx;
        /* linha da caixa: "Nome : ativa" (o resto da linha e comentario) */
        if (fgets(linha, sizeof(linha), f) == NULL) break;
        if (sscanf(linha, "%49s : %d", nomeCaixa, &ativa) != 2) continue;
        cx = CriarCaixa(nomeCaixa, ativa != 0);
        if (cx->ativa) {
            Funcionario *op = ObterFuncionarioLivre(&S->funcionarios);
            if (op != NULL) { cx->operador = op; op->ocupado = true; }
        }
        InserirCaixa(&S->caixas, cx);
        /* numero de clientes desta caixa */
        if (fgets(linha, sizeof(linha), f) == NULL || sscanf(linha, "%d", &nClientes) != 1)
            nClientes = 0;
        for (j = 0; j < nClientes; j++) {
            int idx;
            Cliente *c;
            if (fgets(linha, sizeof(linha), f) == NULL) break;
            if (sscanf(linha, "%49s : %d", nomeCli, &nProd) != 2) continue;
            idx = AdicionarCliente(&S->clientes, nomeCli, nProd);
            if (idx < 0) continue;
            c = &S->clientes.v[idx];
            PrepararCarrinho(c, &S->produtos, S->MAX_PRECO, S->TEMPO_ATENDIMENTO_PRODUTO);
            c->dentroLoja = true;
            c->naFila = true;
            CopiarNome(c->caixaAtual, nomeCaixa);
            EnfileirarCliente(&cx->fila, c);
        }
    }
    fclose(f);
    return 1;
}

/* Carrega tudo dos ficheiros e garante um conjunto minimo de clientes.
   O relogio comeca a hora da abertura da loja (HORA_ABERTURA). */
int InicializarSupermercado(Supermercado *S)
{
    CarregarConfiguracao(S, FICH_CONFIG);
    CarregarProdutos(&S->produtos, FICH_PRODUTOS);
    CarregarFuncionarios(&S->funcionarios, FICH_FUNCIONARIOS);
    CarregarClientes(&S->clientes, FICH_CLIENTES);       /* pool com nomes reais */
    CarregarDados(S, FICH_DADOS);                        /* caixas + clientes iniciais */
    /* se a "pool" ficou pequena (ex.: Clientes.txt em falta), preenche com nomes "Cli#" */
    if (S->clientes.total < POOL_CLIENTES_MIN)
        GerarClientesAleatorios(&S->clientes, POOL_CLIENTES_MIN - S->clientes.total, MAX_CARRINHO);
    /* poe o relogio a hora de abertura (ex.: 08:00) */
    S->relogio->tempoAtual = HORA_ABERTURA * 3600;
    return 1;
}

/* req. 5: abre uma caixa. Reutiliza uma caixa fechada; se nao houver e ainda
   nao se atingiu N_CAIXAS, cria uma nova. Precisa de um operador livre. */
int AbrirNovaCaixa(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS], *fechada = NULL, *nova;
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
        snprintf(nome, MAX_NOME, "Caixa%d", S->caixas.totalCaixas + 1);
        nova = CriarCaixa(nome, true);
        nova->operador = op; op->ocupado = true;
        InserirCaixa(&S->caixas, nova);
        if (S->verboso) printf("  [Caixa aberta] %s (nova)\n", nome);
        return 1;
    }
    return 0;
}

/* req. 7: fecha uma caixa imediatamente e distribui os seus clientes pelas
   restantes caixas abertas. */
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
        destino = EscolherMelhorCaixa(S, cx);
        if (destino != NULL) {
            c = cx->aAtender;
            cx->aAtender = NULL;
            EnfileirarCliente(&destino->fila, c);
            CopiarNome(c->caixaAtual, destino->nome);
        }
    }
    /* move os que estavam em espera */
    while (!FilaVazia(&cx->fila)) {
        c = DesenfileirarCliente(&cx->fila);
        destino = EscolherMelhorCaixa(S, cx);
        if (destino == NULL) { EnfileirarCliente(&cx->fila, c); break; }
        EnfileirarCliente(&destino->fila, c);
        CopiarNome(c->caixaAtual, destino->nome);
    }
    cx->ativa = false;
    cx->aFechar = false;
    if (cx->operador != NULL) { cx->operador->ocupado = false; cx->operador = NULL; }
    printf("Caixa %s fechada e clientes redistribuidos.\n", cx->nome);
    return 1;
}

/* req. 4: muda um cliente que esta em fila para outra caixa. */
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

/* req. 8: diz onde esta uma pessoa (fora, a comprar, em fila ou a ser atendida). */
void PesquisarPessoa(Supermercado *S, char *nomeCliente)
{
    int idx = PesquisarCliente(&S->clientes, nomeCliente);
    Cliente *c;
    Caixa *cx;
    if (idx < 0) { printf("Cliente nao registado.\n"); return; }
    c = &S->clientes.v[idx];
    if (!c->dentroLoja) { printf("%s esta fora da loja.\n", c->nome); return; }
    if (!c->naFila)     { printf("%s esta a fazer compras.\n", c->nome); return; }
    cx = PesquisarCaixa(&S->caixas, c->caixaAtual);
    if (cx != NULL && cx->aAtender == c)
        printf("%s esta a ser atendido na %s.\n", c->nome, c->caixaAtual);
    else
        printf("%s esta em espera na %s.\n", c->nome, c->caixaAtual);
}

/* Mostra o estado atual da loja e de todas as caixas. */
void VerEstadoAtual(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS];
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i, j;
    /* ordena por nome (bubble sort) para uma listagem estavel */
    for (i = 0; i < n - 1; i++)
        for (j = 0; j < n - 1 - i; j++)
            if (strcmp(vec[j]->nome, vec[j + 1]->nome) > 0) {
                Caixa *t = vec[j]; vec[j] = vec[j + 1]; vec[j + 1] = t;
            }
    {
        int t = GetTempo(S->relogio);
        int h = (t / 3600) % 24;
        int m = (t / 60) % 60;
        printf("\n===== Estado da loja '%s' (%02d:%02d - %s) =====\n",
               S->nome, h, m, LojaAberta(S) ? "ABERTA" : "FECHADA");
    }
    printf("Clientes dentro: %d | a fazer compras: %d\n",
           ContarDentroLoja(S), S->emCompras.tamanho);
    for (i = 0; i < n; i++) {
        Caixa *cx = vec[i];
        printf("  %-8s [%-8s] op:%-10s fila:%2d atende:%-10s (atendeu %d, %d prod., %.2f EUR)\n",
               cx->nome,
               cx->ativa ? (cx->aFechar ? "a fechar" : "aberta") : "fechada",
               cx->operador ? cx->operador->nome : "-",
               TamanhoFila(&cx->fila),
               cx->aAtender ? cx->aAtender->nome : "-",
               cx->pessoasAtendidas, cx->produtosVendidos, cx->dinheiroFeito);
    }
    printf("Produtos oferecidos: %d (custo %.2f EUR) | Dinheiro total: %.2f EUR\n",
           S->produtosOferecidos, S->custoOferecido, S->totalDinheiro);
}

/* req. 11: medidas de desempenho do sistema. */
void MedidasDesempenho(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS], *maisPessoas = NULL, *maisProdutos = NULL, *maisDinheiro = NULL;
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i;
    Funcionario *menosOp = NULL;
    printf("\n===== Medidas de Desempenho =====\n");
    for (i = 0; i < n; i++) {
        if (maisPessoas == NULL || vec[i]->pessoasAtendidas > maisPessoas->pessoasAtendidas)
            maisPessoas = vec[i];
        if (maisProdutos == NULL || vec[i]->produtosVendidos > maisProdutos->produtosVendidos)
            maisProdutos = vec[i];
        if (maisDinheiro == NULL || vec[i]->dinheiroFeito > maisDinheiro->dinheiroFeito)
            maisDinheiro = vec[i];
    }
    if (maisPessoas)
        printf("Caixa que atendeu mais pessoas: %s (%d)\n",
               maisPessoas->nome, maisPessoas->pessoasAtendidas);
    if (maisProdutos)
        printf("Caixa que vendeu mais produtos: %s (%d)\n",
               maisProdutos->nome, maisProdutos->produtosVendidos);
    if (maisDinheiro)
        printf("Caixa que fez mais dinheiro: %s (%.2f EUR)\n",
               maisDinheiro->nome, maisDinheiro->dinheiroFeito);
    /* operador que atendeu menos pessoas (entre os que estao ao servico) */
    for (i = 0; i < S->funcionarios.total; i++) {
        Funcionario *fu = &S->funcionarios.v[i];
        if (!fu->ativo || !fu->dentroLoja) continue;
        if (menosOp == NULL || fu->pessoasAtendidas < menosOp->pessoasAtendidas)
            menosOp = fu;
    }
    if (menosOp)
        printf("Operador que atendeu menos pessoas: %s (%d)\n",
               menosOp->nome, menosOp->pessoasAtendidas);
    printf("Produtos oferecidos (o mais barato de cada carrinho): %d | custo total: %.2f EUR\n",
           S->produtosOferecidos, S->custoOferecido);
    printf("Dinheiro total faturado: %.2f EUR\n", S->totalDinheiro);
    if (S->totalAtendidos > 0)
        printf("Tempo medio de espera: %.1f s\n",
               (float) S->somaTemposEspera / S->totalAtendidos);
    printf("\n-- Dinheiro por caixa --\n");
    for (i = 0; i < n; i++)
        printf("  %-8s: %.2f EUR (%d pessoas, %d produtos)\n",
               vec[i]->nome, vec[i]->dinheiroFeito,
               vec[i]->pessoasAtendidas, vec[i]->produtosVendidos);
    printf("\n-- Pessoas atendidas por caixa --\n");
    for (i = 0; i < n; i++)
        ListarAtendidosCaixa(vec[i]);
}

/* req. 9: memoria usada = struct principal + relogio + nos alocados. */
long CalcularMemoriaUtilizada(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS];
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i;
    long total = (long) sizeof(Supermercado) + (long) sizeof(Relogio);
    total += (long) strlen(S->nome) + 1;
    total += (long) S->emCompras.tamanho * (long) sizeof(NoFila);
    for (i = 0; i < n; i++) {
        NoNome *p;
        total += (long) sizeof(Caixa) + (long) sizeof(NoHash);
        total += (long) TamanhoFila(&vec[i]->fila) * (long) sizeof(NoFila);
        for (p = vec[i]->atendidos; p != NULL; p = p->prox)
            total += (long) sizeof(NoNome);
    }
    return total;
}

/* req. 10: memoria desperdicada = espaco reservado nos arrays mas nao usado
   + posicoes vazias da tabela de dispersao. */
long CalcularMemoriaDesperdicada(Supermercado *S)
{
    long desp = 0;
    int i, vazios = 0;
    desp += (long) (MAX_PRODUTOS - S->produtos.total) * (long) sizeof(Produto);
    desp += (long) (MAX_CLIENTES - S->clientes.total) * (long) sizeof(Cliente);
    desp += (long) (MAX_FUNCIONARIOS - S->funcionarios.total) * (long) sizeof(Funcionario);
    for (i = 0; i < TAMANHO_HASH; i++)
        if (S->caixas.tabela[i] == NULL) vazios++;
    desp += (long) vazios * (long) sizeof(NoHash *);
    return desp;
}

void MostrarMemoria(Supermercado *S)
{
    printf("\n===== Memoria =====\n");
    printf("Memoria utilizada:    %ld bytes\n", CalcularMemoriaUtilizada(S));
    printf("Memoria desperdicada: %ld bytes\n", CalcularMemoriaDesperdicada(S));
}

/* req. 2: grava um relatorio com o resumo da simulacao. */
int GravarDados(Supermercado *S, char *ficheiro)
{
    FILE *f = fopen(ficheiro, "w");
    Caixa *vec[MAX_CAIXAS];
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i;
    if (f == NULL) return 0;
    fprintf(f, "=== Relatorio da Simulacao - %s ===\n", S->nome);
    fprintf(f, "Tempo de simulacao: %d s\n", GetTempo(S->relogio));
    fprintf(f, "Clientes atendidos: %d\n", S->totalAtendidos);
    fprintf(f, "Produtos vendidos: %d\n", S->totalProdutosVendidos);
    if (S->totalAtendidos > 0)
        fprintf(f, "Tempo medio de espera: %.1f s\n",
                (float) S->somaTemposEspera / S->totalAtendidos);
    fprintf(f, "Produtos oferecidos (o mais barato de cada carrinho): %d (custo %.2f EUR)\n",
            S->produtosOferecidos, S->custoOferecido);
    fprintf(f, "Dinheiro total faturado: %.2f EUR\n", S->totalDinheiro);
    fprintf(f, "Maximo de caixas abertas em simultaneo: %d\n", S->caixasAbertasMax);
    fprintf(f, "\n-- Por caixa --\n");
    for (i = 0; i < n; i++)
        fprintf(f, "%s: atendidos=%d, produtos=%d, dinheiro=%.2f EUR, operador=%s\n",
                vec[i]->nome, vec[i]->pessoasAtendidas, vec[i]->produtosVendidos,
                vec[i]->dinheiroFeito,
                vec[i]->operador ? vec[i]->operador->nome : "-");
    fclose(f);
    return 1;
}

/* Executa um passo (tick) completo da simulacao. */
void ExecutarPasso(Supermercado *S)
{
    AvancarRelogio(S->relogio);
    EntradaCliente(S);
    AvancarCompras(S);
    AtenderCaixas(S);
    VerificarTemposEspera(S);
    GerirCaixas(S);
}

/* A simulacao "termina" quando nao ha ninguem dentro da loja. */
int SimulacaoTerminada(Supermercado *S)
{
    return ContarDentroLoja(S) == 0;
}

/* Corre 'nPassos' passos. Se comPausa != 0, espera 1 segundo entre passos. */
void ExecutarSimulacao(Supermercado *S, int nPassos, int comPausa)
{
    int i;
    if (ContarCaixasAbertas(S) == 0) AbrirNovaCaixa(S);
    for (i = 0; i < nPassos; i++) {
        ExecutarPasso(S);
        if (comPausa) wait_segundos(1);
    }
}

/* Fecha a "porta" (deixa de aceitar entradas) e atende ate a loja ficar vazia. */
void CorrerAteEsvaziar(Supermercado *S, int comPausa)
{
    int it = 0;
    if (ContarCaixasAbertas(S) == 0) AbrirNovaCaixa(S);
    S->aceitarEntradas = false;
    while (!SimulacaoTerminada(S) && it < MAX_ITERACOES_SIM) {
        ExecutarPasso(S);
        if (comPausa) wait_segundos(1);
        it++;
    }
    S->aceitarEntradas = true;
    printf("Simulacao terminada (loja vazia) ao fim de t = %d s.\n", GetTempo(S->relogio));
}

/* Liberta toda a memoria dinamica. Os arrays-mestre ficam dentro da struct
   principal, por isso sao libertados com ela. */
void DestruirSupermercado(Supermercado *S)
{
    DestruirHashing(&S->caixas);   /* liberta caixas, filas e listas de atendidos */
    DestruirFila(&S->emCompras);   /* liberta os nos da lista de compras */
    DestruirRelogio(S->relogio);
    free(S->nome);
    free(S);
}
