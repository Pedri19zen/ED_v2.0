/* Funcionario.c - operadores de caixa (array) e respetivo menu de gestao */

#include <string.h>
#include "Funcionario.h"

void CriarListaFuncionarios(ListaFuncionarios *L) { L->total = 0; }

/* Adiciona um funcionario. Por omissao fica ao servico e livre. */
int AdicionarFuncionario(ListaFuncionarios *L, char *nome)
{
    Funcionario *f;
    if (L->total >= MAX_FUNCIONARIOS) return -1;
    f = &L->v[L->total];
    f->codigo = 0;        /* sera definido em CarregarFuncionarios se aplicavel */
    CopiarNome(f->nome, nome);
    f->ativo = true;
    f->dentroLoja = true;
    f->ocupado = false;
    f->pessoasAtendidas = 0;
    f->produtosVendidos = 0;
    f->dinheiroFeito = 0;
    L->total++;
    return L->total - 1;
}

int PesquisarFuncionario(ListaFuncionarios *L, char *nome)
{
    int i;
    for (i = 0; i < L->total; i++)
        if (L->v[i].ativo && strcmp(L->v[i].nome, nome) == 0)
            return i;
    return -1;
}

int EditarFuncionario(ListaFuncionarios *L, char *nomeAntigo, char *nomeNovo)
{
    int i = PesquisarFuncionario(L, nomeAntigo);
    if (i < 0) return 0;
    CopiarNome(L->v[i].nome, nomeNovo);
    return 1;
}

/* Desativa um funcionario. Devolve -1 se estiver a operar uma caixa. */
int RemoverFuncionario(ListaFuncionarios *L, char *nome)
{
    int i = PesquisarFuncionario(L, nome);
    if (i < 0) return 0;
    if (L->v[i].ocupado) return -1;
    L->v[i].ativo = false;
    return 1;
}

void ListarFuncionarios(ListaFuncionarios *L)
{
    int i;
    printf("\n--- Funcionarios (%d) ---\n", L->total);
    for (i = 0; i < L->total; i++)
        if (L->v[i].ativo)
            printf("  %-15s | %-10s | atendeu %d pessoas, %d produtos\n",
                   L->v[i].nome, L->v[i].ocupado ? "numa caixa" : "livre",
                   L->v[i].pessoasAtendidas, L->v[i].produtosVendidos);
}

/* Carrega os funcionarios do ficheiro. Aceita dois formatos por linha:
   - "codigo \t nome"      (TSV, formato novo)
   - "nome"                 (linha simples, formato antigo) */
int CarregarFuncionarios(ListaFuncionarios *L, char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");
    char linha[128], nome[MAX_NOME];
    int  codigo, idx;
    if (f == NULL) return 0;
    while (fgets(linha, sizeof(linha), f) != NULL) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (linha[0] == '\0') continue;
        if (sscanf(linha, "%d\t%49[^\r\n]", &codigo, nome) == 2) {
            idx = AdicionarFuncionario(L, nome);
            if (idx >= 0) L->v[idx].codigo = codigo;
        } else {
            AdicionarFuncionario(L, linha);  /* formato antigo: linha = nome */
        }
    }
    fclose(f);
    return 1;
}

int GravarFuncionarios(ListaFuncionarios *L, char *ficheiro)
{
    FILE *f = fopen(ficheiro, "w");
    int i;
    if (f == NULL) return 0;
    for (i = 0; i < L->total; i++)
        if (L->v[i].ativo)
            fprintf(f, "%s\n", L->v[i].nome);
    fclose(f);
    return 1;
}

/* Devolve um funcionario disponivel para abrir uma caixa, ou NULL. */
Funcionario *ObterFuncionarioLivre(ListaFuncionarios *L)
{
    int i;
    for (i = 0; i < L->total; i++)
        if (L->v[i].ativo && L->v[i].dentroLoja && !L->v[i].ocupado)
            return &L->v[i];
    return NULL;
}

/* Submenu de gestao de funcionarios. */
void MenuFuncionarios(ListaFuncionarios *L)
{
    int op, r;
    char nome[MAX_NOME], novo[MAX_NOME];
    do {
        printf("\n=== Gerir Funcionarios ===\n");
        printf("1 - Adicionar\n2 - Editar nome\n3 - Remover\n");
        printf("4 - Pesquisar\n5 - Listar\n0 - Voltar\n");
        op = LerOpcao("Opcao:", 0, 5);
        switch (op) {
            case 1:
                LerString("Nome:", nome, MAX_NOME);
                if (nome[0] == '\0') { printf("Nome invalido.\n"); break; }
                if (PesquisarFuncionario(L, nome) >= 0) { printf("Ja existe.\n"); break; }
                if (AdicionarFuncionario(L, nome) >= 0) {
                    printf("Funcionario adicionado.\n");
                    RegistarHistorico("Adicionou funcionario", nome);
                } else printf("Sem espaco.\n");
                break;
            case 2:
                LerString("Nome atual:", nome, MAX_NOME);
                LerString("Novo nome:", novo, MAX_NOME);
                if (!Confirmar("Confirma a edicao deste funcionario?")) {
                    printf("Operacao cancelada.\n"); break;
                }
                if (EditarFuncionario(L, nome, novo)) {
                    char det[2 * MAX_NOME + 8];
                    snprintf(det, sizeof(det), "%s -> %s", nome, novo);
                    printf("Editado.\n");
                    RegistarHistorico("Editou funcionario", det);
                } else printf("Nao encontrado.\n");
                break;
            case 3:
                LerString("Nome:", nome, MAX_NOME);
                if (!Confirmar("Confirma a remocao deste funcionario?")) {
                    printf("Operacao cancelada.\n"); break;
                }
                r = RemoverFuncionario(L, nome);
                if (r == 1) {
                    printf("Removido.\n");
                    RegistarHistorico("Removeu funcionario", nome);
                }
                else if (r == -1) printf("Esta a operar uma caixa.\n");
                else printf("Nao encontrado.\n");
                break;
            case 4:
                LerString("Nome:", nome, MAX_NOME);
                printf(PesquisarFuncionario(L, nome) >= 0 ? "Encontrado.\n" : "Nao encontrado.\n");
                break;
            case 5: ListarFuncionarios(L); break;
        }
    } while (op != 0);
}
