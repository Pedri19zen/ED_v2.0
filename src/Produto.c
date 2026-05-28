/* Produto.c - catalogo de produtos (array) e respetivo menu de gestao */

#include <string.h>
#include "Produto.h"

void CriarListaProdutos(ListaProdutos *L) { L->total = 0; }

/* Adiciona um produto ao catalogo. Devolve o indice ou -1 se cheio. */
int AdicionarProduto(ListaProdutos *L, char *nome, float preco)
{
    if (L->total >= MAX_PRODUTOS) return -1;
    CopiarNome(L->v[L->total].nome, nome);
    L->v[L->total].preco = preco;
    L->v[L->total].ativo = true;
    L->total++;
    return L->total - 1;
}

/* Procura um produto pelo nome. Devolve o indice ou -1. */
int PesquisarProduto(ListaProdutos *L, char *nome)
{
    int i;
    for (i = 0; i < L->total; i++)
        if (L->v[i].ativo && strcmp(L->v[i].nome, nome) == 0)
            return i;
    return -1;
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

void ListarProdutos(ListaProdutos *L)
{
    int i;
    printf("\n--- Produtos (%d) ---\n", L->total);
    for (i = 0; i < L->total; i++)
        if (L->v[i].ativo)
            printf("  %-20s %8.2f EUR\n", L->v[i].nome, L->v[i].preco);
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

/* Carrega o catalogo do ficheiro (uma linha por produto: "nome;preco"). */
int CarregarProdutos(ListaProdutos *L, char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");
    char linha[128], nome[MAX_NOME];
    float preco;
    if (f == NULL) return 0;
    while (fgets(linha, sizeof(linha), f) != NULL)
        /* le tudo ate ';' como nome e o resto como preco (49 = MAX_NOME-1) */
        if (sscanf(linha, "%49[^;];%f", nome, &preco) == 2)
            AdicionarProduto(L, nome, preco);
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
            fprintf(f, "%s;%.2f\n", L->v[i].nome, L->v[i].preco);
    fclose(f);
    return 1;
}

/* Submenu de gestao do catalogo de produtos. */
void MenuProdutos(ListaProdutos *L)
{
    int op, i;
    char nome[MAX_NOME];
    float preco;
    do {
        printf("\n=== Gerir Produtos ===\n");
        printf("1 - Adicionar\n2 - Editar preco\n3 - Remover\n");
        printf("4 - Pesquisar\n5 - Listar\n0 - Voltar\n");
        op = LerOpcao("Opcao:", 0, 5);
        switch (op) {
            case 1:
                LerString("Nome do produto:", nome, MAX_NOME);
                preco = LerFloat("Preco:");
                if (nome[0] == '\0' || preco <= 0) { printf("Dados invalidos.\n"); break; }
                if (AdicionarProduto(L, nome, preco) >= 0) {
                    printf("Produto adicionado.\n");
                    RegistarHistorico("Adicionou produto");
                } else printf("Catalogo cheio.\n");
                break;
            case 2:
                LerString("Nome do produto:", nome, MAX_NOME);
                preco = LerFloat("Novo preco:");
                if (preco <= 0) { printf("Preco invalido.\n"); break; }
                printf(EditarProduto(L, nome, preco) ? "Editado.\n" : "Nao encontrado.\n");
                break;
            case 3:
                LerString("Nome do produto:", nome, MAX_NOME);
                printf(RemoverProduto(L, nome) ? "Removido.\n" : "Nao encontrado.\n");
                break;
            case 4:
                LerString("Nome do produto:", nome, MAX_NOME);
                i = PesquisarProduto(L, nome);
                if (i >= 0) printf("Encontrado: %s = %.2f EUR\n", L->v[i].nome, L->v[i].preco);
                else printf("Nao encontrado.\n");
                break;
            case 5: ListarProdutos(L); break;
        }
    } while (op != 0);
}
