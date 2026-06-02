/* Simulacao.c - motor da simulacao (passos, picos, e gestao automatica
   das caixas). As acoes invocadas pelo utilizador estao em Acoes.c e os
   relatorios em Relatorios.c. */

#include <string.h>
#include "Supermercado.h"

/* ---- limites internos do simulador ----
   (os parametros configuraveis estao em Configuracao.txt; ver Supermercado.h) */
#define ENTRADAS_POR_TICK_MAX  4       /* tentativas de entrada por tick */
#define MAX_CARRINHO           5       /* artigos maximos de quem entra ao acaso */
#define MAX_ITERACOES_SIM      100000  /* trava de seguranca em CorrerAteEsvaziar */

/* =====================================================================
   Primitivas partilhadas (declaradas em Supermercado.h)
   ===================================================================== */

/* Conta os clientes que estao dentro da loja (a comprar ou em fila). */
int ContarDentroLoja(Supermercado *S)
{
    int i, n = 0;
    for (i = 0; i < S->clientes.total; i++)
        if (S->clientes.v[i].ativo && S->clientes.v[i].dentroLoja) n++;
    return n;
}

/* Conta as caixas que estao abertas. */
int ContarCaixasAbertas(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS];
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i, abertas = 0;
    for (i = 0; i < n; i++)
        if (vec[i]->ativa) abertas++;
    return abertas;
}

/* Escolhe a caixa aberta (e que nao esteja a fechar) com a fila mais curta.
   'excluir' permite ignorar uma caixa (ex.: a que esta a ser fechada). */
Caixa *EscolherMelhorCaixa(Supermercado *S, Caixa *excluir)
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

/* True se a hora atual estiver dentro do horario de funcionamento. */
bool LojaAberta(Supermercado *S)
{
    int hora = (GetTempo(S->relogio) / 3600) % 24;
    return hora >= S->HORA_ABERTURA && hora < S->HORA_FECHO;
}

/* A simulacao "termina" quando nao ha ninguem dentro da loja. */
int SimulacaoTerminada(Supermercado *S)
{
    return ContarDentroLoja(S) == 0;
}

/* =====================================================================
   Helpers internos
   ===================================================================== */

/* Soma o numero de clientes em espera em todas as caixas. */
static int ContarClientesEmFilas(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS];
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i, total = 0;
    for (i = 0; i < n; i++)
        total += TamanhoFila(&vec[i]->fila);
    return total;
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

/* Acrescenta uma linha "    Nome [ X produtos ]\n" ao buffer de entradas
   desde a ultima atualizacao. Se faltar espaco, marca com "    ...\n" e para. */
static void AcrescentarEntrada(Supermercado *S, Cliente *c)
{
    char linha[128];
    int usado = (int) strlen(S->nomesEntradas);
    int restante = (int) sizeof(S->nomesEntradas) - usado;
    snprintf(linha, sizeof(linha), "    %s [ %d produtos ]\n",
             c->nome, c->numProdutos);
    if ((int) strlen(linha) + 1 >= restante) {
        if (restante > 9)
            strcat(S->nomesEntradas, "    ...\n");
        return;
    }
    strcat(S->nomesEntradas, linha);
}

/* =====================================================================
   Passos da simulacao
   ===================================================================== */

/* Passo "entrada": pode entrar mais do que um cliente por passo (durante o
   horario de funcionamento) para manter a loja com movimento. */
static void EntradaCliente(Supermercado *S)
{
    int tentativa;
    if (!S->aceitarEntradas) return;
    if (!LojaAberta(S)) return;                           /* fora de horas */
    for (tentativa = 0; tentativa < ENTRADAS_POR_TICK_MAX; tentativa++) {
        Cliente *c;
        if (Aleatorio(1, 100) > S->CADENCIA_ENTRADA) continue;  /* nao entrou */
        if (ContarDentroLoja(S) >= CAPACIDADE_LOJA) return;    /* loja cheia */
        c = EscolherClienteParaEntrar(S);
        if (c == NULL) return;                                 /* nao ha disponiveis */
        if (c->dentroLoja) continue;                           /* defensivo */
        c->dentroLoja = true;
        c->naFila = false;
        c->caixaAtual[0] = '\0';
        c->numProdutos = Aleatorio(1, MAX_CARRINHO);
        PrepararCarrinho(c, &S->produtos, S->MAX_PRECO, S->TEMPO_ATENDIMENTO_PRODUTO);
        EnfileirarCliente(&S->emCompras, c);
        S->entradasDesdeUpdate++;
        {
            int h = (GetTempo(S->relogio) / 3600) % 24;
            S->entradasPorHora[h]++;
        }
        AcrescentarEntrada(S, c);
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
        cx->operador->dinheiroFeito += pago;
    }
    RegistarAtendido(cx, c->nome);
    /* o cliente sai da loja */
    c->dentroLoja = false;
    c->naFila = false;
    c->caixaAtual[0] = '\0';
    cx->aAtender = NULL;
    S->saidasDesdeUpdate++;
    {
        int h = (GetTempo(S->relogio) / 3600) % 24;
        S->saidasPorHora[h]++;
    }
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

/* Atualiza o pico de clientes dentro da loja e a maior fila observada. */
static void AtualizarPicos(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS];
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i;
    int dentro = ContarDentroLoja(S);
    if (dentro > S->dentroLojaPico) {
        S->dentroLojaPico = dentro;
        S->horaPico = GetTempo(S->relogio);
    }
    for (i = 0; i < n; i++) {
        int f = TamanhoFila(&vec[i]->fila);
        if (f > S->maxFilaObservada) S->maxFilaObservada = f;
    }
}

/* =====================================================================
   API publica
   ===================================================================== */

void ExecutarPasso(Supermercado *S)
{
    AvancarRelogio(S->relogio);
    EntradaCliente(S);
    AvancarCompras(S);
    AtenderCaixas(S);
    VerificarTemposEspera(S);
    GerirCaixas(S);
    AtualizarPicos(S);
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

/* Reinicia um "novo dia": evacua todos os clientes, esvazia as filas,
   limpa os contadores e poe o relogio na hora de abertura. As caixas
   mantem-se com o estado actual (abertas/fechadas) e os funcionarios
   continuam ao servico. */
void IniciarNovoDia(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS];
    int n, i;
    /* clientes -> todos fora da loja */
    for (i = 0; i < S->clientes.total; i++) {
        Cliente *c = &S->clientes.v[i];
        c->dentroLoja = false;
        c->naFila = false;
        c->caixaAtual[0] = '\0';
        c->tempoEspera = 0;
        c->foiOferecido = false;
    }
    /* caixas -> esvazia filas, lista de atendidos e zera estatisticas */
    n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS);
    for (i = 0; i < n; i++) {
        Caixa *cx = vec[i];
        NoNome *p, *seg;
        while (!FilaVazia(&cx->fila)) DesenfileirarCliente(&cx->fila);
        cx->aAtender = NULL;
        cx->aFechar = false;
        cx->pessoasAtendidas = 0;
        cx->produtosVendidos = 0;
        cx->dinheiroFeito = 0;
        for (p = cx->atendidos; p != NULL; p = seg) { seg = p->prox; free(p); }
        cx->atendidos = NULL;
    }
    /* lista global de quem anda as compras */
    while (!FilaVazia(&S->emCompras)) DesenfileirarCliente(&S->emCompras);
    /* funcionarios -> zera estatisticas (mantem ocupado/livre) */
    for (i = 0; i < S->funcionarios.total; i++) {
        S->funcionarios.v[i].pessoasAtendidas = 0;
        S->funcionarios.v[i].produtosVendidos = 0;
        S->funcionarios.v[i].dinheiroFeito = 0;
    }
    /* contadores globais e historico do dia */
    S->totalAtendidos = 0;
    S->totalProdutosVendidos = 0;
    S->somaTemposEspera = 0;
    S->produtosOferecidos = 0;
    S->custoOferecido = 0;
    S->totalDinheiro = 0;
    S->caixasAbertasMax = 0;
    S->maxFilaObservada = 0;
    S->dentroLojaPico = 0;
    S->horaPico = 0;
    S->entradasDesdeUpdate = 0;
    S->saidasDesdeUpdate = 0;
    S->nomesEntradas[0] = '\0';
    for (i = 0; i < 24; i++) {
        S->entradasPorHora[i] = 0;
        S->saidasPorHora[i] = 0;
    }
    /* relogio na hora de abertura */
    S->relogio->tempoAtual = S->HORA_ABERTURA * 3600;
    printf("Novo dia iniciado as %02d:00.\n", S->HORA_ABERTURA);
}
