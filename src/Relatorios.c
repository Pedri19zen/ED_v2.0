/* Relatorios.c - saidas para o utilizador, medidas de desempenho, contabilidade
   da memoria e gravacao do relatorio final (requisitos 2, 9, 10 e 11). */

#include <string.h>
#include "Supermercado.h"

/* Converte "Caixa1" em "Caixa 1" para uma listagem mais legivel. */
static void FormatarNomeCaixa(char *destino, char *original)
{
    if (strncmp(original, "Caixa", 5) == 0 && original[5] != '\0' && original[5] != ' ')
        snprintf(destino, MAX_NOME, "Caixa %s", original + 5);
    else
        snprintf(destino, MAX_NOME, "%s", original);
}

/* Mostra o estado atual da loja e de todas as caixas no formato de "frame"
   da simulacao automatica (cabecalho com hora + caixas em colunas). */
void VerEstadoAtual(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS];
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i, j;
    int t, h, m, sec;
    char nomeFmt[MAX_NOME];
    /* ordena por nome (bubble sort) para uma listagem estavel */
    for (i = 0; i < n - 1; i++)
        for (j = 0; j < n - 1 - i; j++)
            if (strcmp(vec[j]->nome, vec[j + 1]->nome) > 0) {
                Caixa *tmp = vec[j]; vec[j] = vec[j + 1]; vec[j + 1] = tmp;
            }
    t = GetTempo(S->relogio);
    h = (t / 3600) % 24;
    m = (t / 60) % 60;
    sec = t % 60;
    printf("\n%s[%02d:%02d:%02d] ---------- ESTADO DAS CAIXAS -------------------------%s\n",
           COR_HDR, h, m, sec, COR_RESET);
    printf("  Clientes na loja: %d\n", ContarDentroLoja(S));
    printf("  Desde a ultima atualizacao: %d entraram | %d sairam\n",
           S->entradasDesdeUpdate, S->saidasDesdeUpdate);
    printf("%s----------------------------------------------------%s\n", COR_DIM, COR_RESET);
    for (i = 0; i < n; i++) {
        Caixa *cx = vec[i];
        FormatarNomeCaixa(nomeFmt, cx->nome);
        if (cx->ativa) {
            printf("  %-8s %s[ABERTA]%s | ", nomeFmt, COR_OK, COR_RESET);
            ImprimirBarraFila(TamanhoFila(&cx->fila));
            printf(" | A atender: %s\n",
                   cx->aAtender ? cx->aAtender->nome : "(livre)");
        } else {
            printf("  %-8s %s[FECHADA]%s\n", nomeFmt, COR_DIM, COR_RESET);
        }
    }
    printf("%s----------------------------------------------------%s\n", COR_DIM, COR_RESET);
    if (S->nomesEntradas[0] != '\0') {
        printf("  Entraram:\n%s", S->nomesEntradas);
    }
    /* reinicia os contadores para a proxima atualizacao */
    S->entradasDesdeUpdate = 0;
    S->saidasDesdeUpdate = 0;
    S->nomesEntradas[0] = '\0';
}

/* req. 11: medidas de desempenho do sistema. */
void MedidasDesempenho(Supermercado *S)
{
    Caixa *vec[MAX_CAIXAS], *maisPessoas = NULL, *maisProdutos = NULL, *maisDinheiro = NULL;
    int n = ObterTodasCaixas(&S->caixas, vec, MAX_CAIXAS), i;
    Funcionario *menosOp = NULL, *opMaisDinheiro = NULL;
    int hpico_h = (S->horaPico / 3600) % 24;
    int hpico_m = (S->horaPico / 60) % 60;
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
    for (i = 0; i < S->funcionarios.total; i++) {
        Funcionario *fu = &S->funcionarios.v[i];
        if (!fu->ativo || !fu->dentroLoja) continue;
        if (menosOp == NULL || fu->pessoasAtendidas < menosOp->pessoasAtendidas)
            menosOp = fu;
        if (opMaisDinheiro == NULL || fu->dinheiroFeito > opMaisDinheiro->dinheiroFeito)
            opMaisDinheiro = fu;
    }
    if (menosOp)
        printf("Operador que atendeu menos pessoas: %s (%d)\n",
               menosOp->nome, menosOp->pessoasAtendidas);
    if (opMaisDinheiro)
        printf("Operador que mais dinheiro fez: %s (%.2f EUR)\n",
               opMaisDinheiro->nome, opMaisDinheiro->dinheiroFeito);
    printf("Produtos oferecidos (o mais barato de cada carrinho): %d | custo total: %.2f EUR\n",
           S->produtosOferecidos, S->custoOferecido);
    printf("Dinheiro total faturado: %.2f EUR\n", S->totalDinheiro);
    if (S->totalAtendidos > 0)
        printf("Tempo medio de espera: %.1f s\n",
               (float) S->somaTemposEspera / S->totalAtendidos);
    printf("Maior fila observada: %d pessoas\n", S->maxFilaObservada);
    printf("Pico de clientes na loja: %d (as %02d:%02d)\n",
           S->dentroLojaPico, hpico_h, hpico_m);
    printf("\n-- Dinheiro por caixa --\n");
    for (i = 0; i < n; i++)
        printf("  %-8s: %.2f EUR (%d pessoas, %d produtos)\n",
               vec[i]->nome, vec[i]->dinheiroFeito,
               vec[i]->pessoasAtendidas, vec[i]->produtosVendidos);
    printf("\n-- Pessoas atendidas por caixa --\n");
    for (i = 0; i < n; i++)
        ListarAtendidosCaixa(vec[i]);
}

/* req. 9: memoria usada = struct principal + relogio + nos alocados nas
   listas/filas/hashes dinamicos. */
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
    total += (long) S->clientes.idxNome.totalEntradas * (long) sizeof(NoHashNome);
    total += (long) S->produtos.idxNome.totalEntradas * (long) sizeof(NoHashNome);
    return total;
}

/* req. 10: memoria desperdicada = espaco reservado nos arrays mas nao usado
   + posicoes vazias das tabelas de dispersao. */
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
    vazios = 0;
    for (i = 0; i < TAMANHO_HASH_NOMES; i++)
        if (S->clientes.idxNome.tabela[i] == NULL) vazios++;
    desp += (long) vazios * (long) sizeof(NoHashNome *);
    vazios = 0;
    for (i = 0; i < TAMANHO_HASH_NOMES; i++)
        if (S->produtos.idxNome.tabela[i] == NULL) vazios++;
    desp += (long) vazios * (long) sizeof(NoHashNome *);
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
    int hpico_h, hpico_m;
    Funcionario *opMaisDinheiro = NULL;
    if (f == NULL) return 0;
    hpico_h = (S->horaPico / 3600) % 24;
    hpico_m = (S->horaPico / 60) % 60;
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
    fprintf(f, "Maior fila observada: %d pessoas\n", S->maxFilaObservada);
    fprintf(f, "Pico de clientes dentro da loja: %d (as %02d:%02d)\n",
            S->dentroLojaPico, hpico_h, hpico_m);

    for (i = 0; i < S->funcionarios.total; i++) {
        Funcionario *fu = &S->funcionarios.v[i];
        if (!fu->ativo) continue;
        if (opMaisDinheiro == NULL || fu->dinheiroFeito > opMaisDinheiro->dinheiroFeito)
            opMaisDinheiro = fu;
    }
    if (opMaisDinheiro)
        fprintf(f, "Operador que mais dinheiro fez: %s (%.2f EUR)\n",
                opMaisDinheiro->nome, opMaisDinheiro->dinheiroFeito);

    fprintf(f, "\n-- Por caixa --\n");
    for (i = 0; i < n; i++)
        fprintf(f, "%s: atendidos=%d, produtos=%d, dinheiro=%.2f EUR, operador=%s\n",
                vec[i]->nome, vec[i]->pessoasAtendidas, vec[i]->produtosVendidos,
                vec[i]->dinheiroFeito,
                vec[i]->operador ? vec[i]->operador->nome : "-");

    fprintf(f, "\n-- Movimento por hora --\n");
    fprintf(f, "  hora   entraram   sairam\n");
    for (i = 0; i < 24; i++)
        if (S->entradasPorHora[i] > 0 || S->saidasPorHora[i] > 0)
            fprintf(f, "  %02d:00   %5d     %5d\n", i,
                    S->entradasPorHora[i], S->saidasPorHora[i]);
    fclose(f);
    return 1;
}
