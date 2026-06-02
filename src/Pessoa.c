/* Pessoa.c - clientes do supermercado (array) e respetivo menu de gestao */

#include <string.h>
#include "Pessoa.h"

void CriarListaClientes(ListaClientes *L)
{
    L->total = 0;
    CriarHashingNomes(&L->idxNome);
}

void DestruirListaClientes(ListaClientes *L)
{
    DestruirHashingNomes(&L->idxNome);
    L->total = 0;
}

/* Adiciona um cliente registado (ainda fora da loja). Devolve indice ou -1. */
int AdicionarCliente(ListaClientes *L, char *nome, int numProdutos)
{
    Cliente *c;
    if (L->total >= MAX_CLIENTES) return -1;
    c = &L->v[L->total];
    c->codigo = 0;                  /* sera definido por CarregarClientes, se houver */
    CopiarNome(c->nome, nome);
    c->numProdutos = numProdutos;
    c->valorCarrinho = 0;
    c->precoMenorProduto = 0;
    c->tempoAtendimento = 0;
    c->tempoRestante = 0;
    c->tempoEspera = 0;
    c->tempoComprasRestante = 0;
    c->ativo = true;
    c->dentroLoja = false;
    c->naFila = false;
    c->foiOferecido = false;
    c->caixaAtual[0] = '\0';
    L->total++;
    /* indexa o nome (aponta para o buffer no array-mestre) */
    InserirNomeHash(&L->idxNome, c->nome, L->total - 1);
    return L->total - 1;
}

/* Pesquisa por nome em O(1) media usando a tabela de dispersao.
   So devolve indice se o cliente ainda estiver "ativo". */
int PesquisarCliente(ListaClientes *L, char *nome)
{
    int idx = PesquisarNomeHash(&L->idxNome, nome);
    if (idx < 0 || idx >= L->total) return -1;
    if (!L->v[idx].ativo) return -1;
    return idx;
}

int EditarCliente(ListaClientes *L, char *nome, int numProdutos)
{
    int i = PesquisarCliente(L, nome);
    if (i < 0) return 0;
    L->v[i].numProdutos = numProdutos;
    return 1;
}

/* Desativa um cliente. Devolve -1 se ele ainda estiver dentro da loja. */
int RemoverCliente(ListaClientes *L, char *nome)
{
    int i = PesquisarCliente(L, nome);
    if (i < 0) return 0;
    if (L->v[i].dentroLoja) return -1;
    L->v[i].ativo = false;
    return 1;
}

/* Lista os clientes registados (limitada a 50 linhas para nao inundar o ecra). */
void ListarClientes(ListaClientes *L)
{
    int i, mostrados = 0;
    printf("\n--- Clientes registados (%d) ---\n", L->total);
    for (i = 0; i < L->total; i++) {
        if (!L->v[i].ativo) continue;
        printf("  %-15s | artigos: %2d | %s\n",
               L->v[i].nome, L->v[i].numProdutos,
               L->v[i].dentroLoja ? (L->v[i].naFila ? "em fila" : "as compras") : "fora");
        if (++mostrados >= 50) { printf("  ... (%d no total)\n", L->total); break; }
    }
}

/* Carrega a "pool" de clientes do ficheiro. Formato (TAB-separado):
   codigo \t nome
   As terminacoes \r\n (Windows) sao tratadas. Devolve 1 se conseguiu abrir. */
int CarregarClientes(ListaClientes *L, char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");
    char linha[128], nome[MAX_NOME];
    int codigo, idx;
    if (f == NULL) return 0;
    while (fgets(linha, sizeof(linha), f) != NULL) {
        /* %[^\t\r\n] le o nome ate ao tab/CR/LF */
        if (sscanf(linha, "%d\t%49[^\t\r\n]", &codigo, nome) == 2) {
            idx = AdicionarCliente(L, nome, 0);   /* numProdutos definido na entrada */
            if (idx >= 0) L->v[idx].codigo = codigo;
        }
    }
    fclose(f);
    return 1;
}

/* Cria clientes ficticios, util para testar com muitos dados. */
void GerarClientesAleatorios(ListaClientes *L, int quantos, int maxProdutos)
{
    char nome[MAX_NOME];
    int i;
    for (i = 0; i < quantos; i++) {
        snprintf(nome, MAX_NOME, "Cli%d", L->total + 1);
        if (AdicionarCliente(L, nome, Aleatorio(1, maxProdutos)) < 0) break;
    }
}

/* Prepara o carrinho do cliente: para cada artigo (numProdutos) escolhe um
   produto do catalogo e soma o preco; o tempo de passagem de cada produto e
   um valor em [2, TEMPO_ATENDIMENTO_PRODUTO]. */
void PrepararCarrinho(Cliente *c, ListaProdutos *prods, int maxPreco, int tempoAtendProduto)
{
    int k, idx;
    float preco, somaPagar = 0.0f, somaComprar = 0.0f;
    if (c->numProdutos < 1) c->numProdutos = 1;
    c->valorCarrinho = 0;
    c->precoMenorProduto = -1;   /* sentinela: ainda nao ha artigos */
    for (k = 0; k < c->numProdutos; k++) {
        idx = (prods != NULL) ? ProdutoAleatorio(prods) : -1;
        if (idx >= 0) {
            /* usa os tempos especificos do produto */
            preco        = prods->v[idx].preco;
            somaPagar   += prods->v[idx].tempoPagar;
            somaComprar += prods->v[idx].tempoComprar;
        } else {
            /* sem catalogo: usa valores aleatorios como fallback */
            preco        = (float) Aleatorio(1, maxPreco);
            somaPagar   += (float) Aleatorio(2, tempoAtendProduto);
            somaComprar += (float) Aleatorio(2, tempoAtendProduto);
        }
        c->valorCarrinho += preco;
        /* guarda o preco do artigo mais barato (sera o oferecido se preciso) */
        if (c->precoMenorProduto < 0 || preco < c->precoMenorProduto)
            c->precoMenorProduto = preco;
    }
    if (c->precoMenorProduto < 0) c->precoMenorProduto = 0;
    /* arredonda os tempos para segundos inteiros; garante pelo menos 1 segundo */
    c->tempoAtendimento     = (int)(somaPagar   + 0.5f);
    c->tempoComprasRestante = (int)(somaComprar + 0.5f);
    if (c->tempoAtendimento     < 1) c->tempoAtendimento = 1;
    if (c->tempoComprasRestante < 1) c->tempoComprasRestante = 1;
    c->tempoRestante = c->tempoAtendimento;
    c->tempoEspera = 0;
    c->foiOferecido = false;
}

/* Submenu de gestao de clientes. */
void MenuClientes(ListaClientes *L)
{
    int op, n, i, r;
    char nome[MAX_NOME];
    do {
        printf("\n=== Gerir Clientes ===\n");
        printf("1 - Adicionar\n2 - Editar nr. de produtos\n3 - Remover\n");
        printf("4 - Pesquisar\n5 - Listar\n6 - Gerar aleatorios\n0 - Voltar\n");
        op = LerOpcao("Opcao:", 0, 6);
        switch (op) {
            case 1:
                LerString("Nome:", nome, MAX_NOME);
                n = LerInteiro("Nr. de produtos:");
                if (nome[0] == '\0' || n < 1) { printf("Dados invalidos.\n"); break; }
                if (PesquisarCliente(L, nome) >= 0) { printf("Ja existe.\n"); break; }
                if (AdicionarCliente(L, nome, n) >= 0) {
                    printf("Cliente adicionado.\n");
                    RegistarHistorico("Adicionou cliente", nome);
                } else printf("Sem espaco.\n");
                break;
            case 2:
                LerString("Nome:", nome, MAX_NOME);
                n = LerInteiro("Novo nr. de produtos:");
                if (n < 1) { printf("Valor invalido.\n"); break; }
                if (!Confirmar("Confirma a edicao deste cliente?")) {
                    printf("Operacao cancelada.\n"); break;
                }
                if (EditarCliente(L, nome, n)) {
                    printf("Editado.\n");
                    RegistarHistorico("Editou cliente", nome);
                } else printf("Nao encontrado.\n");
                break;
            case 3:
                LerString("Nome:", nome, MAX_NOME);
                if (!Confirmar("Confirma a remocao deste cliente?")) {
                    printf("Operacao cancelada.\n"); break;
                }
                r = RemoverCliente(L, nome);
                if (r == 1) {
                    printf("Removido.\n");
                    RegistarHistorico("Removeu cliente", nome);
                }
                else if (r == -1) printf("Esta na loja, nao pode ser removido agora.\n");
                else printf("Nao encontrado.\n");
                break;
            case 4:
                LerString("Nome:", nome, MAX_NOME);
                i = PesquisarCliente(L, nome);
                if (i >= 0) printf("Encontrado: %s (%d artigos)\n", L->v[i].nome, L->v[i].numProdutos);
                else printf("Nao encontrado.\n");
                break;
            case 5: ListarClientes(L); break;
            case 6:
                n = LerInteiro("Quantos clientes gerar?");
                if (n > 0) {
                    char det[32];
                    GerarClientesAleatorios(L, n, 10);
                    snprintf(det, sizeof(det), "%d clientes", n);
                    printf("Gerados.\n");
                    RegistarHistorico("Gerou clientes aleatorios", det);
                } else printf("Valor invalido.\n");
                break;
        }
    } while (op != 0);
}
