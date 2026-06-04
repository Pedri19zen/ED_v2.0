# Estruturas de dados e dependências

Este documento corresponde ao requisito do enunciado:
*"A ilustração das dependências entre as várias entidades (estruturas) criadas"*.

## Visão geral

```mermaid
classDiagram
    direction LR

    class Supermercado {
        +char* nome
        +ListaProdutos produtos
        +ListaClientes clientes
        +ListaFuncionarios funcionarios
        +Fila emCompras
        +Hashing caixas
        +Relogio* relogio
        +configuracao (MAX_ESPERA, ...)
        +estatisticas (totais, picos)
    }

    class ListaProdutos {
        +Produto v[MAX_PRODUTOS]
        +int total
        +HashingNomes idxNome
    }
    class ListaClientes {
        +Cliente v[MAX_CLIENTES]
        +int total
        +HashingNomes idxNome
    }
    class ListaFuncionarios {
        +Funcionario v[MAX_FUNCIONARIOS]
        +int total
    }

    class Produto {
        +int codigo
        +char nome[128]
        +float preco
        +float tempoComprar
        +float tempoPagar
        +bool ativo
    }
    class Cliente {
        +int codigo
        +char nome[50]
        +int numProdutos
        +float valorCarrinho
        +int tempo*
        +bool ativo / dentroLoja / naFila
        +char caixaAtual[50]
    }
    class Funcionario {
        +int codigo
        +char nome[50]
        +bool ativo / dentroLoja / ocupado
        +int pessoasAtendidas
        +int produtosVendidos
        +float dinheiroFeito
    }

    class Caixa {
        +char nome[50]
        +bool ativa / aFechar
        +Fila fila
        +Cliente* aAtender
        +Funcionario* operador
        +NoNome* atendidos
        +int pessoasAtendidas
        +float dinheiroFeito
    }

    class Fila {
        +NoFila* inicio
        +NoFila* fim
        +int tamanho
    }
    class NoFila {
        +Cliente* cliente
        +NoFila* prox
    }
    class NoNome {
        +char nome[50]
        +NoNome* prox
    }

    class Hashing {
        +NoHash* tabela[13]
        +int totalCaixas
    }
    class NoHash {
        +Caixa* caixa
        +NoHash* prox
    }

    class HashingNomes {
        +NoHashNome* tabela[1009]
        +int totalEntradas
    }
    class NoHashNome {
        +char* nome
        +int indice
        +NoHashNome* prox
    }

    class Relogio {
        +int tempoAtual
        +int velocidade
    }

    Supermercado *-- ListaProdutos
    Supermercado *-- ListaClientes
    Supermercado *-- ListaFuncionarios
    Supermercado *-- Fila : emCompras
    Supermercado *-- Hashing : caixas
    Supermercado --> Relogio

    ListaProdutos *-- Produto : v[]
    ListaProdutos *-- HashingNomes : idxNome
    ListaClientes *-- Cliente : v[]
    ListaClientes *-- HashingNomes : idxNome
    ListaFuncionarios *-- Funcionario : v[]

    Fila *-- NoFila
    NoFila --> Cliente : pointer

    Hashing *-- NoHash
    NoHash --> Caixa : owns

    Caixa *-- Fila
    Caixa *-- NoNome : atendidos
    Caixa --> Funcionario : operador (pointer)
    Caixa --> Cliente : aAtender (pointer)

    HashingNomes *-- NoHashNome
    NoHashNome --> Produto : refers
    NoHashNome --> Cliente : refers
```

## Notação

- `*--`: **composição** (a estrutura pai contém / é dona da memória da filha).
- `-->`: **referência** (ponteiro/índice; o pai não liberta a filha).

Pelas regras de Doxygen, esta convenção corresponde a *"strong containment"*
para os arrays-mestre e listas ligadas; e a *"weak association"* para os
ponteiros que apenas referenciam objectos que vivem noutro array.

## Quem aloca/liberta o quê

| Estrutura | Onde está alocada | Quem liberta |
|-----------|-------------------|--------------|
| `Supermercado` | `malloc` em `CriarSupermercado` | `DestruirSupermercado` |
| Arrays-mestre `v[]` (produtos, clientes, funcionarios) | dentro do `Supermercado` (stack-like) | libertados com o `Supermercado` |
| `Caixa` (cada uma) | `malloc` em `CriarCaixa` | `DestruirCaixa` (via `DestruirHashing`) |
| Nós da `Fila` (`NoFila`) | `malloc` em `EnfileirarCliente` | `DesenfileirarCliente`/`DestruirFila` |
| Nós dos atendidos (`NoNome`) | `malloc` em `RegistarAtendido` | `DestruirCaixa` |
| Nós do hash de caixas (`NoHash`) | `malloc` em `InserirCaixa` | `DestruirHashing` |
| Nós do hash de nomes (`NoHashNome`) | `malloc` em `InserirNomeHash` | `DestruirHashingNomes` |
| `Relogio` | `malloc` em `CriarRelogio` | `DestruirRelogio` |

Os nós da fila **não libertam** os `Cliente` para onde apontam — esses vivem
no array-mestre dentro do `Supermercado`. Da mesma forma os `NoHashNome`
não copiam o nome: guardam apenas o ponteiro para o buffer que vive no
array-mestre.

## Resumo das ED escolhidas

| ED | Onde se usa | Porquê |
|----|-------------|--------|
| Array | produtos, clientes, funcionarios | acesso O(1) por índice; tamanho conhecido após carregar os ficheiros |
| Lista ligada (queue) | filas das caixas + "em compras" | inserir no fim e remover do início em O(1), sem deslocar elementos |
| Lista ligada simples | atendidos por caixa | só se insere e percorre |
| Tabela de dispersão | caixas, índices de nome (clientes/produtos) | pesquisa O(1) média; substitui a varredura linear sobre 10.000+ entradas |

Tudo respeita o enunciado: "*Deverá usar listas/filas e hashing ou árvores*".
Não foram usadas árvores, grafos ou heaps.

## Fluxo da simulação (passo a passo)

```mermaid
flowchart LR
    A[AvancarRelogio] --> B[EntradaCliente]
    B --> C[AvancarCompras]
    C --> D[AtenderCaixas]
    D --> E[VerificarTemposEspera]
    E --> F[GerirCaixas]
    F --> G[AtualizarPicos]
    G --> A
```

Cada passo consume `velocidade` segundos do relógio lógico. A função
`ExecutarPasso` corre os 7 estágios pela ordem indicada.

## Diagrama dos módulos (.c)

```mermaid
flowchart TB
    main --> Supermercado
    main --> Uteis
    Supermercado --> Simulacao
    Supermercado --> Acoes
    Supermercado --> Relatorios
    Simulacao --> Acoes
    Acoes --> Pessoa
    Acoes --> Caixa
    Relatorios --> Caixa
    Pessoa --> HashingNomes
    Produto --> HashingNomes
    Caixa --> Fila
    Caixa --> Funcionario
    Hashing --> Caixa
    Fila --> Pessoa
```
