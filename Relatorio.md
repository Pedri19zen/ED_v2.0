# Relatório — Projeto de Estruturas de Dados 2025/2026
## "Gestão de Caixas de um Supermercado"

**Escola Superior de Tecnologia e Gestão de Viseu — Engenharia Informática**

> Identificação dos alunos: ver ficheiro `Alunos.txt`.

---

## 1. Introdução

O programa simula e faz a gestão do conjunto de caixas de atendimento do supermercado
*Compra Aqui Lda.*. Permite à gerência acompanhar o funcionamento das caixas e decidir
quando abrir/fechar caixas, de modo a **minimizar o número de caixas abertas** mantendo o
**tempo médio de espera** dos clientes abaixo de `MAX_ESPERA`.

O programa arranca num **menu principal** (a simulação só corre por opção do utilizador) e
está dividido em módulos, cada um com a sua estrutura de dados e respetivas operações.

## 2. Tempo de desenvolvimento

Tempo aproximado de execução do projeto: **cerca de 35–40 horas** (análise do enunciado,
desenho das estruturas, implementação, testes e relatório). *(Ajustar ao esforço real do grupo.)*

## 3. Estruturas de dados utilizadas

Foram usadas apenas estruturas das aulas de ED: **arrays**, **listas ligadas / filas**,
**hashing** e **ficheiros**.

### 3.1. Dados-mestre em arrays (acesso rápido, carregados uma vez)

| Estrutura | Onde | Porquê array |
|---|---|---|
| `Produto` em `ListaProdutos` | `Produto.h` | catálogo fixo, acesso direto por índice |
| `Cliente` em `ListaClientes` | `Pessoa.h` | conjunto de clientes registados (pool da simulação) |
| `Funcionario` em `ListaFuncionarios` | `Funcionario.h` | operadores de caixa |

Cada "lista" é `{ T v[MAX]; int total; }`. Para **remover** desativa-se o registo
(`ativo = false`), mantendo os índices/ponteiros estáveis durante a simulação.

Os três arrays-mestre carregam-se de **ficheiros TAB-separados** com identificadores reais:
- `data/Produtos.txt` — `codigo \t nome \t preco \t tempoComprar \t tempoPagar` (~9 662 produtos).
- `data/Clientes.txt` — `codigo \t nome` (~10 000 clientes).
- `data/Funcionarios.txt` — `codigo \t nome` (cerca de 90 funcionarios; aceita também o
  formato antigo só com o nome).

Os limites estão dimensionados para o ficheiro completo (`MAX_PRODUTOS=10000`,
`MAX_CLIENTES=12000`, `MAX_FUNCIONARIOS=200`) e os nomes de produtos usam
`MAX_NOME_PRODUTO=128` (vão até ~106 caracteres).

O `Cliente` tem campos booleanos que controlam o seu estado e **evitam estados inválidos**
(entrar duas vezes, sair sem ter entrado):

```c
typedef struct {
    char nome[MAX_NOME]; int numProdutos; float valorCarrinho;
    int tempoAtendimento, tempoRestante, tempoEspera;
    bool ativo;        // registo existe
    bool dentroLoja;   // está dentro do supermercado
    bool naFila;       // está numa fila de uma caixa
    bool foiOferecido; // já recebeu um produto grátis
    char caixaAtual[MAX_NOME];
} Cliente;
```

### 3.2. Listas ligadas / filas (estado dinâmico da simulação)

- `Fila` (`Fila.h`) — fila FIFO (lista ligada) de `Cliente*`. É usada:
  - na **lista de compras** (`emCompras`): clientes que entraram mas ainda andam às compras;
  - em **cada caixa**: clientes em espera para serem atendidos.
- `NoNome` (`Caixa.h`) — pequena lista ligada com os nomes dos clientes já atendidos por cada
  caixa (suporta "listar pessoas atendidas por uma caixa").

### 3.3. Hashing das caixas (`Hashing.h`)

As caixas ficam numa **tabela de dispersão** com chave = nome da caixa e resolução de
colisões por **encadeamento**. A função de dispersão soma os códigos das letras módulo o
tamanho da tabela. Permite **pesquisar uma caixa pelo nome rapidamente** e o programa escala
para muitas caixas.

## 4. Dependências entre as entidades

```
                         Supermercado  (estrutura central)
        ┌───────────────┬──────┴───────┬────────────────┐
   arrays-mestre    emCompras        Hashing          Relogio
   ┌────┼────┐      (Fila)           (caixas)
Produto Cliente Funcionario             │
                                       Caixa ── Fila (espera) ── Cliente*
                                         │
                                      Funcionario (operador da caixa)

   Uteis  →  usado por todos (constantes globais + funções auxiliares)
```

- `Supermercado` agrega tudo: os três arrays-mestre, a lista `emCompras`, o `Hashing` das
  caixas, o `Relogio` e os parâmetros de configuração + estatísticas globais.
- Uma `Caixa` contém uma `Fila` de `Cliente*` e aponta para o seu `Funcionario` (operador).
- As filas guardam **ponteiros** para os clientes do array-mestre (sem duplicar dados).

## 5. Funcionalidades implementadas (mapa para o enunciado)

| # | Requisito | Onde |
|---|---|---|
| 1 | Carregar dados dos ficheiros para memória | `InicializarSupermercado`, `Carregar*` |
| 2 | Gravar dados / medidas (tempo, atendidos, espera média…) | `GravarDados` → `resultado.txt` |
| 3 | Histórico em `*.csv` das ações do utilizador | `RegistarHistorico` → `historico.csv` |
| 4 | Cliente muda de caixa | `MoverClienteEntreCaixas` |
| 5 | Abrir caixa (manual + automático se média > `MAX_FILA`) | `AbrirNovaCaixa`, `GerirCaixas` |
| 6 | Fechar a caixa com menos pessoas se média < `MIN_FILA` | `GerirCaixas` (fecho gracioso) |
| 7 | Fecho forçado + redistribuição dos clientes | `FecharCaixaImediato` |
| 8 | Pesquisar pessoa (em que caixa está) | `PesquisarPessoa` |
| 9 | Memória utilizada | `CalcularMemoriaUtilizada` |
| 10 | Memória desperdiçada | `CalcularMemoriaDesperdicada` |
| 11 | Medidas finais (caixa que atendeu mais pessoas, mais produtos, **mais dinheiro**, operador que atendeu menos, produtos oferecidos e respetivo custo, dinheiro por caixa) | `MedidasDesempenho` |

## 6. Funcionamento da simulação

A simulação avança por **passos (ticks)**; cada passo corresponde a `velocidade` segundos.
Em cada passo (`ExecutarPasso`):

1. **Relógio** avança (arranca às `08:00`; a loja só aceita entradas até `20:00`).
2. **Entrada** de clientes (até `ENTRADAS_POR_TICK_MAX` por passo, cada com probabilidade
   `cadenciaEntrada`%): respeita `CAPACIDADE_LOJA`, nunca deixa um cliente entrar duas
   vezes, e só durante o horário (`HORA_ABERTURA`-`HORA_FECHO`). O carrinho é gerado com
   produtos aleatórios do catálogo.
3. **Compras → caixa**: quem termina as compras vai para a **fila mais curta** entre as
   caixas abertas.
4. **Atendimento**: cada caixa atende o cliente da frente (decrementa `tempoRestante`, que é
   a soma dos `tempoPagar` dos artigos do carrinho); os restantes acumulam `tempoEspera`.
   Ao terminar, o cliente sai e atualizam-se as estatísticas.
5. **Garantia de qualidade**: se `tempoEspera > MAX_ESPERA`, oferece-se ao cliente o
   **produto mais barato** do seu carrinho (regista-se o nº de produtos oferecidos e o custo);
   o dinheiro faturado pela caixa é o valor do carrinho menos esse produto oferecido.
6. **Gestão de caixas**: abre/fecha caixas conforme a média de clientes por fila.

Ao escolher **Iniciar simulação**, esta corre **automaticamente**, mostrando o estado a cada
~2 segundos, até o utilizador premir uma tecla. Aí abre-se um menu de pausa que permite
**continuar**, **saltar X segundos**, **avançar 1 passo**, **ver o estado** ou **terminar**.
O estado mostra, por caixa, quem está a ser atendido, o tamanho da fila e o **dinheiro feito**.

## 7. Gestão de memória

Os arrays-mestre estão dentro da estrutura `Supermercado` (uma única alocação). Apenas os nós
das listas/filas/hash, o relógio e a própria estrutura são alocados dinamicamente. Toda a
memória é libertada em `DestruirSupermercado` (sem fugas). O programa também calcula a memória
utilizada e a desperdiçada (requisitos 9 e 10).

## 8. Organização das pastas, compilação e execução

```
include/  -> ficheiros .h            obj/  -> objetos .o   (gerado)
src/      -> ficheiros .c            bin/  -> executavel   (gerado)
data/     -> ficheiros de dados .txt
Makefile  Relatorio.md  Alunos.txt
```

A partir da raiz do projeto (Windows com MSYS2 usa `mingw32-make`; em Linux/Mac usa `make`):

```
mingw32-make          # compila -> bin/supermercado
mingw32-make run      # compila e executa
mingw32-make clean    # apaga os ficheiros gerados
```

Em alternativa, sem Makefile: `gcc -Wall -Iinclude -o supermercado src/*.c`.
Ou abrir `Projeto_ED_25_26.cbp` no Code::Blocks. Compila sem erros nem avisos (`-Wall -Wextra`).

> Executar **a partir da raiz** do projeto, para que os ficheiros de entrada em
> `data/` sejam encontrados. Os ficheiros de saída (`historico.csv`, `resultado.txt`)
> são criados nessa raiz.

## 9. Assunções

- O catálogo de produtos e a lista de funcionários não constam do enunciado: foram criados os
  ficheiros `Produtos.txt` e `Funcionarios.txt` (geríveis por menu).
- `Dados.txt` indica o **número de produtos** por cliente; nome/preço de cada artigo são
  sorteados do catálogo, com preço em `]0, MAX_PRECO]` e tempo de passagem em
  `[2, TEMPO_ATENDIMENTO_PRODUTO]`.
- Os tempos estão em segundos; a simulação é discreta (1 passo = `velocidade` segundos).
- "Dentro da loja" = a fazer compras **ou** numa fila; `CAPACIDADE_LOJA` limita os clientes
  simultâneos.
- Memória desperdiçada = espaço reservado nos arrays fixos mas não usado + posições vazias da
  tabela de dispersão.
- O custo de um produto oferecido é estimado como um artigo de valor médio do carrinho.
