/**
 * @file Pessoa.c
 * @brief Clientes do supermercado (array) com pesquisa O(1) por nome.
 */

#include <string.h>
#include "Pessoa.h"

/** @brief Inicializa a lista vazia e o indice de nomes. */
void CriarListaClientes(ListaClientes *L)
{
    L->total = 0;
    CriarHashingNomes(&L->idxNome);
}

/** @brief Liberta o indice de nomes (o array v[] vive na struct mae). */
void DestruirListaClientes(ListaClientes *L)
{
    DestruirHashingNomes(&L->idxNome);
    L->total = 0;
}

/**
 * @brief Adiciona um cliente registado (ainda fora da loja).
 * @return Indice no array ou -1 se cheio.
 */
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

/**
 * @brief Pesquisa por nome em O(1) media usando a tabela de dispersao.
 *
 * So devolve indice se o cliente ainda estiver "activo".
 */
int PesquisarCliente(ListaClientes *L, char *nome)
{
    int idx = PesquisarNomeHash(&L->idxNome, nome);
    if (idx < 0 || idx >= L->total) return -1;
    if (!L->v[idx].ativo) return -1;
    return idx;
}

/** @brief Verifica se 'prefixo' e' prefixo de 'str' ignorando maiusculas. */
static int PrefixoIgualSemCaso(char *str, char *prefixo)
{
    int i;
    for (i = 0; prefixo[i] != '\0'; i++) {
        if (str[i] == '\0') return 0;
        if (ToMaiscula(str[i]) != ToMaiscula(prefixo[i])) return 0;
    }
    return 1;
}

/**
 * @brief Pesquisa tolerante: match exacto via hash, ou prefixo sem caso.
 *
 * Primeiro tenta match exacto (rapido, via hash); se nao encontrar, faz
 * um varrimento linear case-insensitive com prefixo. Devolve o primeiro
 * cliente activo que comece pelas mesmas letras (qualquer caixa).
 */
int PesquisarClienteTolerante(ListaClientes *L, char *texto)
{
    int idx = PesquisarCliente(L, texto);
    int i;
    if (idx >= 0) return idx;
    if (texto[0] == '\0') return -1;
    for (i = 0; i < L->total; i++) {
        if (!L->v[i].ativo) continue;
        if (PrefixoIgualSemCaso(L->v[i].nome, texto))
            return i;
    }
    return -1;
}

/** @brief Edita o numero de produtos do carrinho de um cliente. */
int EditarCliente(ListaClientes *L, char *nome, int numProdutos)
{
    int i = PesquisarCliente(L, nome);
    if (i < 0) return 0;
    L->v[i].numProdutos = numProdutos;
    return 1;
}

/**
 * @brief Desactiva um cliente.
 * @return 1 ok, -1 se ainda estiver dentro da loja, 0 nao encontrado.
 */
int RemoverCliente(ListaClientes *L, char *nome)
{
    int i = PesquisarCliente(L, nome);
    if (i < 0) return 0;
    if (L->v[i].dentroLoja) return -1;
    L->v[i].ativo = false;
    return 1;
}

/**
 * @brief Lista os clientes registados em paginas de 20.
 *
 * O utilizador pode pedir a pagina seguinte (Enter) ou sair (q).
 */
void ListarClientes(ListaClientes *L)
{
    int i = 0, naPagina;
    char resp[8];
    printf("\n--- Clientes registados (%d) ---\n", L->total);
    while (i < L->total) {
        naPagina = 0;
        while (i < L->total && naPagina < 20) {
            if (L->v[i].ativo) {
                printf("  %-30s | artigos: %2d | %s\n",
                       L->v[i].nome, L->v[i].numProdutos,
                       L->v[i].dentroLoja ? (L->v[i].naFila ? "em fila" : "as compras") : "fora");
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

/**
 * @brief Carrega a "pool" de clientes do ficheiro.
 *
 * Formato (TAB-separado): "codigo TAB nome". As terminacoes \\r\\n
 * (Windows) sao tratadas.
 * @return 1 se conseguiu abrir, 0 caso contrario.
 */
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

/** @brief Cria clientes ficticios "Cli#", util para testar com muitos dados. */
void GerarClientesAleatorios(ListaClientes *L, int quantos, int maxProdutos)
{
    char nome[MAX_NOME];
    int i;
    for (i = 0; i < quantos; i++) {
        snprintf(nome, MAX_NOME, "Cli%d", L->total + 1);
        if (AdicionarCliente(L, nome, Aleatorio(1, maxProdutos)) < 0) break;
    }
}

/**
 * @brief Prepara o carrinho do cliente.
 *
 * Para cada artigo (numProdutos) escolhe um produto do catalogo e soma
 * o preco; o tempo de passagem de cada produto e' um valor em
 * [2, TEMPO_ATENDIMENTO_PRODUTO]. Se 'prods' for NULL usa valores
 * aleatorios como fallback.
 */
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

/** @brief Submenu interactivo de gestao de clientes. */
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
