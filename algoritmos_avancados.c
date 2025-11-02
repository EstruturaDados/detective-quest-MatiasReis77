#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NOME 64
#define MAX_PISTA 128
#define HASH_SIZE 101   // tamanho da tabela hash (primo simples)

/* ----------------------------
   ESTRUTURAS
   ----------------------------*/

// Nó da árvore de salas (mapa da mansão)
typedef struct Sala {
    char nome[MAX_NOME];
    char pista[MAX_PISTA]; // pista associada à sala (pode ser string vazia)
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// Nó da BST de pistas coletadas
typedef struct PistaNode {
    char pista[MAX_PISTA];
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

// Entrada da lista encadeada para tabela hash (mapeia pista -> suspeito)
typedef struct HashEntry {
    char pista[MAX_PISTA];
    char suspeito[MAX_NOME];
    struct HashEntry *proximo;
} HashEntry;

// Tabela hash (vetor de ponteiros para HashEntry)
typedef struct HashTable {
    HashEntry *buckets[HASH_SIZE];
} HashTable;

/* ----------------------------
   PROTÓTIPOS
   ----------------------------*/

// Funções de criação / manipulação de salas
Sala* criarSala(const char *nome); // cria dinamicamente um cômodo
void conectarSalas(Sala *pai, Sala *esq, Sala *dir);
void liberarSalas(Sala *raiz);

// Funções BST de pistas
PistaNode* inserirPista(PistaNode *raiz, const char *pista); // insere pista na BST
void liberarPistas(PistaNode *raiz);
void listarPistasInOrder(PistaNode *raiz);

// Funções de tabela hash
HashTable* criarHash();
void inserirNaHash(HashTable *h, const char *pista, const char *suspeito); // insere associação pista->suspeito
const char* encontrarSuspeito(HashTable *h, const char *pista); // consulta suspeito por pista
void liberarHash(HashTable *h);

// Funções de jogo / lógica
void explorarSalas(Sala *inicio, PistaNode **arvorePistas, HashTable *h); // navega e coleta pistas
int verificarSuspeitoFinal(PistaNode *arvorePistas, HashTable *h, const char *acusado); // verifica se ≥2 pistas apontam para acusado

/* ----------------------------
   IMPLEMENTAÇÃO
   ----------------------------*/

// Função de hash simples (djb2)
static unsigned long hash_string(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash % HASH_SIZE;
}

/*
 criarSala() – cria dinamicamente um cômodo.
 Recebe o nome e cria sala com pista vazia (pista pode ser setada por strcpy externamente).
*/
Sala* criarSala(const char *nome) {
    Sala *n = (Sala*) malloc(sizeof(Sala));
    if (!n) {
        fprintf(stderr, "Erro de alocação ao criar sala.\n");
        exit(1);
    }
    strncpy(n->nome, nome, MAX_NOME-1);
    n->nome[MAX_NOME-1] = '\0';
    n->pista[0] = '\0';
    n->esquerda = n->direita = NULL;
    return n;
}

// conecta filhos à sala pai
void conectarSalas(Sala *pai, Sala *esq, Sala *dir) {
    if (!pai) return;
    pai->esquerda = esq;
    pai->direita = dir;
}

// libera memória da árvore de salas
void liberarSalas(Sala *raiz) {
    if (!raiz) return;
    liberarSalas(raiz->esquerda);
    liberarSalas(raiz->direita);
    free(raiz);
}

/*
 inserirPista() / adicionarPista() – insere a pista coletada na árvore de pistas (BST)
 Evita duplicatas; mantém ordem alfabética por strcmp.
*/
PistaNode* inserirPista(PistaNode *raiz, const char *pista) {
    if (raiz == NULL) {
        PistaNode *n = (PistaNode*) malloc(sizeof(PistaNode));
        if (!n) {
            fprintf(stderr, "Erro de alocação ao inserir pista.\n");
            exit(1);
        }
        strncpy(n->pista, pista, MAX_PISTA-1);
        n->pista[MAX_PISTA-1] = '\0';
        n->esquerda = n->direita = NULL;
        return n;
    }
    int cmp = strcmp(pista, raiz->pista);
    if (cmp < 0)
        raiz->esquerda = inserirPista(raiz->esquerda, pista);
    else if (cmp > 0)
        raiz->direita = inserirPista(raiz->direita, pista);
    // se igual, ignora (dup)
    return raiz;
}

void listarPistasInOrder(PistaNode *raiz) {
    if (!raiz) return;
    listarPistasInOrder(raiz->esquerda);
    printf(" - %s\n", raiz->pista);
    listarPistasInOrder(raiz->direita);
}

void liberarPistas(PistaNode *raiz) {
    if (!raiz) return;
    liberarPistas(raiz->esquerda);
    liberarPistas(raiz->direita);
    free(raiz);
}

/*
 inserirNaHash() – insere associação pista/suspeito na tabela hash.
 Usa chaining (listas encadeadas) para colisões.
*/
HashTable* criarHash() {
    HashTable *h = (HashTable*) malloc(sizeof(HashTable));
    if (!h) { fprintf(stderr, "Erro de alocacao da hash.\n"); exit(1); }
    for (int i = 0; i < HASH_SIZE; ++i) h->buckets[i] = NULL;
    return h;
}

void inserirNaHash(HashTable *h, const char *pista, const char *suspeito) {
    unsigned long idx = hash_string(pista);
    // verifica se já existe entrada com mesma pista -> atualiza suspeito (ou ignora)
    HashEntry *cur = h->buckets[idx];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0) {
            // atualiza suspeito se desejar; aqui mantemos o primeiro inserido (não atualizamos)
            return;
        }
        cur = cur->proximo;
    }
    // cria nova entrada
    HashEntry *e = (HashEntry*) malloc(sizeof(HashEntry));
    if (!e) { fprintf(stderr, "Erro de alocacao HashEntry.\n"); exit(1); }
    strncpy(e->pista, pista, MAX_PISTA-1);
    e->pista[MAX_PISTA-1] = '\0';
    strncpy(e->suspeito, suspeito, MAX_NOME-1);
    e->suspeito[MAX_NOME-1] = '\0';
    e->proximo = h->buckets[idx];
    h->buckets[idx] = e;
}

/*
 encontrarSuspeito() – consulta o suspeito correspondente a uma pista.
 Retorna ponteiro para string interna (não deve ser free'd pelo chamador) ou NULL se não achar.
*/
const char* encontrarSuspeito(HashTable *h, const char *pista) {
    unsigned long idx = hash_string(pista);
    HashEntry *cur = h->buckets[idx];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0)
            return cur->suspeito;
        cur = cur->proximo;
    }
    return NULL;
}

void liberarHash(HashTable *h) {
    if (!h) return;
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashEntry *cur = h->buckets[i];
        while (cur) {
            HashEntry *tmp = cur;
            cur = cur->proximo;
            free(tmp);
        }
        h->buckets[i] = NULL;
    }
    free(h);
}

/*
 explorarSalas() – navega pela árvore e ativa o sistema de pistas.
 - Ao visitar uma sala exibe o nome.
 - Se existir pista, exibe e insere na BST de pistas coletadas.
 - Permite 'e' (esquerda), 'd' (direita) ou 's' (sair).
*/
void explorarSalas(Sala *inicio, PistaNode **arvorePistas, HashTable *h) {
    if (!inicio) return;
    Sala *atual = inicio;
    char escolha;
    // Usaremos loop até o jogador escolher sair (s)
    while (1) {
        printf("\nVocê está em: %s\n", atual->nome);
        if (strlen(atual->pista) > 0) {
            printf("  >> Você encontrou uma pista: \"%s\"\n", atual->pista);
            // adiciona à BST (evita duplicatas)
            *arvorePistas = inserirPista(*arvorePistas, atual->pista);
            // (opcional) informar a quem a pista aponta — mas por design só mostraremos no julgamento
        } else {
            printf("  (Não há pista nesta sala)\n");
        }

        // Mostrar opções
        printf("\nCaminhos disponíveis:\n");
        if (atual->esquerda) printf(" (e) Ir para %s (esquerda)\n", atual->esquerda->nome);
        if (atual->direita)  printf(" (d) Ir para %s (direita)\n", atual->direita->nome);
        printf(" (s) Sair e ir ao julgamento\n");
        printf("Escolha: ");
        scanf(" %c", &escolha);

        if (escolha == 'e' && atual->esquerda) {
            atual = atual->esquerda;
        } else if (escolha == 'd' && atual->direita) {
            atual = atual->direita;
        } else if (escolha == 's') {
            printf("\nEncerrando exploração. Levando as pistas ao julgamento...\n");
            break;
        } else {
            printf("Opção inválida ou caminho inexistente. Tente novamente.\n");
        }
    }
}

/*
 verificarSuspeitoFinal() – conduz à fase de julgamento final.
 Percorre a BST de pistas coletadas e conta quantas pistas (distintas) apontam para o acusado.
 Retorna o número de pistas que apontam para o acusado.
*/
static int contarPistasParaSuspeitoRec(PistaNode *raiz, HashTable *h, const char *acusado) {
    if (!raiz) return 0;
    int cont = 0;
    // in-order traversal (ordem não importante para contagem)
    cont += contarPistasParaSuspeitoRec(raiz->esquerda, h, acusado);
    const char *sus = encontrarSuspeito(h, raiz->pista);
    if (sus != NULL && strcmp(sus, acusado) == 0) cont++;
    cont += contarPistasParaSuspeitoRec(raiz->direita, h, acusado);
    return cont;
}

int verificarSuspeitoFinal(PistaNode *arvorePistas, HashTable *h, const char *acusado) {
    // conta quantas pistas coletadas apontam para 'acusado'
    int cont = contarPistasParaSuspeitoRec(arvorePistas, h, acusado);
    return cont;
}

/* ----------------------------
   MAIN: montagem do mapa, hash de pistas/suspeitos e fluxo do jogo
   ----------------------------*/

int main() {
    // --- montar mapa fixo da mansão (árvore de salas) ---
    Sala *hall = criarSala("Hall de Entrada");
    Sala *salaEstar = criarSala("Sala de Estar");
    Sala *cozinha = criarSala("Cozinha");
    Sala *biblioteca = criarSala("Biblioteca");
    Sala *jardim = criarSala("Jardim");
    Sala *porao = criarSala("Porão");
    Sala *torre = criarSala("Torre");

    // associar pistas às salas (regras codificadas)
    strncpy(hall->pista, "Pegadas molhadas no tapete", MAX_PISTA-1);
    strncpy(salaEstar->pista, "Relógio parado às 3h15", MAX_PISTA-1);
    strncpy(cozinha->pista, "Copo quebrado com resíduo de vinho", MAX_PISTA-1);
    strncpy(biblioteca->pista, "Página rasgada de um romance", MAX_PISTA-1);
    strncpy(jardim->pista, "", MAX_PISTA-1); // sem pista
    strncpy(porao->pista, "Cofre trancado com senha danificada", MAX_PISTA-1);
    strncpy(torre->pista, "Fibra de tecido preto presa na janela", MAX_PISTA-1);

    // construir ligações (árvore binária)
    conectarSalas(hall, salaEstar, cozinha);
    conectarSalas(salaEstar, biblioteca, jardim);
    conectarSalas(cozinha, porao, torre);

    // --- montar tabela hash de pista -> suspeito ---
    HashTable *tabela = criarHash();
    // Inserir associações (pista -> suspeito)
    inserirNaHash(tabela, "Pegadas molhadas no tapete", "Sr. Almeida");
    inserirNaHash(tabela, "Relógio parado às 3h15", "Sra. Helena");
    inserirNaHash(tabela, "Copo quebrado com resíduo de vinho", "Sra. Helena");
    inserirNaHash(tabela, "Página rasgada de um romance", "Prof. Braga");
    inserirNaHash(tabela, "Cofre trancado com senha danificada", "Sr. Almeida");
    inserirNaHash(tabela, "Fibra de tecido preto presa na janela", "Suspeito Desconhecido");
    // (Note: jardim não tem pista associada no mapa)

    // --- BST de pistas coletadas (vazia no início) ---
    PistaNode *arvorePistas = NULL;

    // Mensagem inicial
    printf("=== DETECTIVE QUEST: Julgamento Final ===\n");
    printf("Você começa sua investigação no Hall de Entrada.\n");

    // exploração interativa
    explorarSalas(hall, &arvorePistas, tabela);

    // Exibir pistas coletadas
    printf("\n==========================\n");
    printf("Pistas coletadas (ordenadas):\n");
    if (arvorePistas == NULL) {
        printf(" (nenhuma pista coletada)\n");
    } else {
        listarPistasInOrder(arvorePistas);
    }
    printf("==========================\n");

    // Solicitar acusação
    char acusado[MAX_NOME];
    printf("\nInforme o nome do suspeito que deseja acusar (ex.: 'Sra. Helena'): ");
    // limpar buffer antes de fgets
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
    if (fgets(acusado, sizeof(acusado), stdin) != NULL) {
        // remover newline
        size_t ln = strlen(acusado);
        if (ln > 0 && acusado[ln-1] == '\n') acusado[ln-1] = '\0';
    } else {
        acusado[0] = '\0';
    }

    if (strlen(acusado) == 0) {
        printf("Nenhum suspeito informado. Encerrando sem julgamento.\n");
    } else {
        int count = verificarSuspeitoFinal(arvorePistas, tabela, acusado);
        printf("\nVocê acusou: %s\n", acusado);
        printf("Pistas coletadas que apontam para %s: %d\n", acusado, count);
        if (count >= 2) {
            printf("\n>>> VEREDICTO: Há evidências suficientes! %s é considerado(a) culpado(a).\n", acusado);
        } else {
            printf("\n>>> VEREDICTO: Evidências insuficientes. %s NÃO pode ser considerado(a) culpado(a).\n", acusado);
        }
    }

    // liberar tudo
    liberarSalas(hall);
    liberarPistas(arvorePistas);
    liberarHash(tabela);

    printf("\nObrigado por jogar Detective Quest — Julgamento Final!\n");
    return 0;
}
