/* Produto.c - catalogo de produtos (array) e respetivo menu de gestao */

#include <string.h>
#include "Produto.h"

void CriarListaProdutos(ListaProdutos *L)
{
    L->total = 0;
    CriarHashingNomes(&L->idxNome);
}

void DestruirListaProdutos(ListaProdutos *L)
{
    DestruirHashingNomes(&L->idxNome);
    L->total = 0;
}

/* Adiciona um produto ao catalogo. Devolve o indice ou -1 se cheio. */
int AdicionarProduto(ListaProdutos *L, int codigo, char *nome,
                     float preco, float tempoComprar, float tempoPagar)
{
    Produto *p;
    if (L->total >= MAX_PRODUTOS) return -1;
    p = &L->v[L->total];
    p->codigo = codigo;
    /* nome de produto pode ser longo: usamos snprintf com o tamanho proprio */
    snprintf(p->nome, MAX_NOME_PRODUTO, "%s", nome);
    p->preco = preco;
    p->tempoComprar = tempoComprar;
    p->tempoPagar = tempoPagar;
    p->ativo = true;
    L->total++;
    InserirNomeHash(&L->idxNome, p->nome, L->total - 1);
    return L->total - 1;
}

/* Pesquisa por nome em O(1) media via tabela de dispersao. */
int PesquisarProduto(ListaProdutos *L, char *nome)
{
    int idx = PesquisarNomeHash(&L->idxNome, nome);
    if (idx < 0 || idx >= L->total) return -1;
    if (!L->v[idx].ativo) return -1;
    return idx;
}

int EditarProduto(ListaProdutos *L, char *nome, float novoPreco)
{
    int i = PesquisarProduto(L, nome);
    if (i < 0) return 0;
    L->v[i].preco = novoPreco;
    return 1;
}

/* "Remover" = desativar, para manter o array (e os indices) estaveis. */
int RemoverProduto(ListaProdutos *L, char *nome)
{
    int i = PesquisarProduto(L, nome);
    if (i < 0) return 0;
    L->v[i].ativo = false;
    return 1;
}

/* Mostra o catalogo em paginas de 20 linhas. O utilizador escolhe se quer
   ver mais (Enter) ou sair (q). */
void ListarProdutos(ListaProdutos *L)
{
    int i = 0, naPagina;
    char resp[8];
    printf("\n--- Produtos (%d) ---\n", L->total);
    printf("  %-6s %-50s %8s %8s %8s\n", "codigo", "nome", "preco", "comprar", "pagar");
    while (i < L->total) {
        naPagina = 0;
        while (i < L->total && naPagina < 20) {
            if (L->v[i].ativo) {
                printf("  %-6d %-50.50s %8.2f %8.1f %8.1f\n",
                       L->v[i].codigo, L->v[i].nome,
                       L->v[i].preco, L->v[i].tempoComprar, L->v[i].tempoPagar);
                naPagina++;
            }
            i++;
        }
        if (i < L->total) {
            printf("  --- Enter para mais, 'q' para sair: ");
            fflush(stdout);
            if (fgets(resp, sizeof(resp), stdin) == NULL) break;
            if (resp[0] == 'q' || resp[0] == 'Q') break;
        }
    }
}

/* Devolve o indice de um produto ativo escolhido ao acaso (ou -1). */
int ProdutoAleatorio(ListaProdutos *L)
{
    int i, tentativa;
    if (L->total == 0) return -1;
    /* tenta algumas posicoes ao acaso; se falhar, procura o 1o ativo */
    for (tentativa = 0; tentativa < 5; tentativa++) {
        i = Aleatorio(0, L->total - 1);
        if (L->v[i].ativo) return i;
    }
    for (i = 0; i < L->total; i++)
        if (L->v[i].ativo) return i;
    return -1;
}

/* Carrega o catalogo do ficheiro. Formato (uma linha por produto, separado
   por TAB): codigo \t nome \t preco \t tempoComprar \t tempoPagar */
int CarregarProdutos(ListaProdutos *L, char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");
    char linha[512];
    char nome[MAX_NOME_PRODUTO];
    int   codigo;
    float preco, tComprar, tPagar;
    if (f == NULL) return 0;
    while (fgets(linha, sizeof(linha), f) != NULL) {
        /* %127[^\t] le o nome (com espacos) ate ao tabulador seguinte */
        if (sscanf(linha, "%d\t%127[^\t]\t%f\t%f\t%f",
                   &codigo, nome, &preco, &tComprar, &tPagar) == 5)
            AdicionarProduto(L, codigo, nome, preco, tComprar, tPagar);
    }
    fclose(f);
    return 1;
}

int GravarProdutos(ListaProdutos *L, char *ficheiro)
{
    FILE *f = fopen(ficheiro, "w");
    int i;
    if (f == NULL) return 0;
    for (i = 0; i < L->total; i++)
        if (L->v[i].ativo)
            fprintf(f, "%d\t%s\t%.2f\t%.1f\t%.1f\n",
                    L->v[i].codigo, L->v[i].nome, L->v[i].preco,
                    L->v[i].tempoComprar, L->v[i].tempoPagar);
    fclose(f);
    return 1;
}

/* Submenu de gestao do catalogo de produtos. */
void MenuProdutos(ListaProdutos *L)
{
    int op, i;
    char nome[MAX_NOME_PRODUTO];
    float preco, tComprar, tPagar;
    do {
        printf("\n=== Gerir Produtos ===\n");
        printf("1 - Adicionar\n2 - Editar preco\n3 - Remover\n");
        printf("4 - Pesquisar\n5 - Listar\n0 - Voltar\n");
        op = LerOpcao("Opcao:", 0, 5);
        switch (op) {
            case 1:
                LerString("Nome do produto:", nome, MAX_NOME_PRODUTO);
                preco    = LerFloat("Preco:");
                tComprar = LerFloat("Tempo de compra (s):");
                tPagar   = LerFloat("Tempo de pagamento (s):");
                if (nome[0] == '\0' || preco <= 0 || tComprar < 0 || tPagar < 0) {
                    printf("Dados invalidos.\n"); break;
                }
                /* codigo automatico: ultimo + 1, ou 90001 se vazio */
                {
                    int codigo = (L->total > 0) ? (L->v[L->total - 1].codigo + 1) : 90001;
                    if (AdicionarProduto(L, codigo, nome, preco, tComprar, tPagar) >= 0) {
                        printf("Produto adicionado (codigo %d).\n", codigo);
                        RegistarHistorico("Adicionou produto", nome);
                    } else printf("Catalogo cheio.\n");
                }
                break;
            case 2:
                LerString("Nome do produto:", nome, MAX_NOME_PRODUTO);
                preco = LerFloat("Novo preco:");
                if (preco <= 0) { printf("Preco invalido.\n"); break; }
                if (!Confirmar("Confirma a edicao deste produto?")) {
                    printf("Operacao cancelada.\n"); break;
                }
                if (EditarProduto(L, nome, preco)) {
                    printf("Editado.\n");
                    RegistarHistorico("Editou produto", nome);
                } else printf("Nao encontrado.\n");
                break;
            case 3:
                LerString("Nome do produto:", nome, MAX_NOME_PRODUTO);
                if (!Confirmar("Confirma a remocao deste produto?")) {
                    printf("Operacao cancelada.\n"); break;
                }
                if (RemoverProduto(L, nome)) {
                    printf("Removido.\n");
                    RegistarHistorico("Removeu produto", nome);
                } else printf("Nao encontrado.\n");
                break;
            case 4:
                LerString("Nome do produto:", nome, MAX_NOME_PRODUTO);
                i = PesquisarProduto(L, nome);
                if (i >= 0)
                    printf("Encontrado: %d %s %.2f EUR (comprar %.1fs, pagar %.1fs)\n",
                           L->v[i].codigo, L->v[i].nome, L->v[i].preco,
                           L->v[i].tempoComprar, L->v[i].tempoPagar);
                else printf("Nao encontrado.\n");
                break;
            case 5: ListarProdutos(L); break;
        }
    } while (op != 0);
}
