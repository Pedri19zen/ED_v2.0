#ifndef HASHING_NOMES_H_INCLUDED
#define HASHING_NOMES_H_INCLUDED

#include "Uteis.h"

/* =====================================================================
   HashingNomes.h - tabela de dispersao genérica que mapeia um nome para
   o indice da entrada no array-mestre correspondente (clientes, produtos,
   etc.). Permite pesquisas O(1) media, em vez do varrimento linear.

   Notas:
   - As colisoes sao resolvidas por encadeamento (lista ligada por bucket).
   - Guardamos apenas um ponteiro para o nome original (que vive no array
     -mestre e nao muda); evita copiar a string e suporta nomes longos.
   ===================================================================== */

#define TAMANHO_HASH_NOMES 1009   /* primo, ~10 entradas/bucket com 10k itens */

typedef struct NoHashNome {
    char *nome;                /* aponta para o nome no array-mestre */
    int   indice;              /* indice nesse array */
    struct NoHashNome *prox;
} NoHashNome;

typedef struct {
    NoHashNome *tabela[TAMANHO_HASH_NOMES];
    int totalEntradas;
} HashingNomes;

void CriarHashingNomes(HashingNomes *h);
void InserirNomeHash(HashingNomes *h, char *nome, int indice);
int  PesquisarNomeHash(HashingNomes *h, char *nome);   /* indice ou -1 */
void DestruirHashingNomes(HashingNomes *h);

#endif /* HASHING_NOMES_H_INCLUDED */
