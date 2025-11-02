#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// ===============================================
/// ESTRUTURAS DE DADOS
/// ===============================================

// Estrutura que representa um cômodo da mansão (nó da árvore binária)
typedef struct Sala {
    char nome[50];
    char pista[100];
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// Estrutura que representa um nó da árvore BST de pistas coletadas
typedef struct PistaNode {
    char pista[100];
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

/// ===============================================
/// FUNÇÕES DE CRIAÇÃO
/// ===============================================

// Cria dinamicamente uma nova sala com nome e (opcionalmente) uma pista
Sala* criarSala(const char *nome, const char *pista) {
    Sala *nova = (Sala*) malloc(sizeof(Sala));
    if (nova == NULL) {
        printf("Erro ao alocar memória para a sala.\n");
        exit(1);
    }
    strcpy(nova->nome, nome);
    if (pista != NULL)
        strcpy(nova->pista, pista);
    else
        strcpy(nova->pista, ""); // sem pista
    nova->esquerda = NULL;
    nova->direita = NULL;
    return nova;
}

// Cria um novo nó de pista para a BST
PistaNode* criarPistaNode(const char *pista) {
    PistaNode *novo = (PistaNode*) malloc(sizeof(PistaNode));
    if (novo == NULL) {
        printf("Erro ao alocar memória para a pista.\n");
        exit(1);
    }
    strcpy(novo->pista, pista);
    novo->esquerda = NULL;
    novo->direita = NULL;
    return novo;
}

/// ===============================================
/// FUNÇÕES DE INSERÇÃO E EXIBIÇÃO DE PISTAS
/// ===============================================

// Insere uma nova pista na BST em ordem alfabética
PistaNode* inserirPista(PistaNode *raiz, const char *pista) {
    if (raiz == NULL)
        return criarPistaNode(pista);

    if (strcmp(pista, raiz->pista) < 0)
        raiz->esquerda = inserirPista(raiz->esquerda, pista);
    else if (strcmp(pista, raiz->pista) > 0)
        raiz->direita = inserirPista(raiz->direita, pista);
    // se for igual, ignora (evita duplicatas)

    return raiz;
}

// Exibe as pistas coletadas em ordem alfabética (in-order traversal)
void exibirPistas(PistaNode *raiz) {
    if (raiz == NULL) return;
    exibirPistas(raiz->esquerda);
    printf("🔎 %s\n", raiz->pista);
    exibirPistas(raiz->direita);
}

/// ===============================================
/// FUNÇÃO DE EXPLORAÇÃO DA MANSÃO
/// ===============================================

// Permite ao jogador explorar a mansão e coletar pistas
void explorarSalasComPistas(Sala *atual, PistaNode **arvorePistas) {
    char escolha;

    while (1) {
        printf("\nVocê está em: %s\n", atual->nome);

        // Se há uma pista na sala, adiciona à árvore BST
        if (strlen(atual->pista) > 0) {
            printf("💡 Você encontrou uma pista: \"%s\"\n", atual->pista);
            *arvorePistas = inserirPista(*arvorePistas, atual->pista);
        } else {
            printf("Nenhuma pista nesta sala.\n");
        }

        // Exibe caminhos possíveis
        printf("\nEscolha um caminho:\n");
        if (atual->esquerda != NULL)
            printf(" (e) Ir para %s à esquerda\n", atual->esquerda->nome);
        if (atual->direita != NULL)
            printf(" (d) Ir para %s à direita\n", atual->direita->nome);
        printf(" (s) Sair da mansão\n");
        printf("Sua escolha: ");
        scanf(" %c", &escolha);

        if (escolha == 'e' && atual->esquerda != NULL) {
            atual = atual->esquerda;
        } else if (escolha == 'd' && atual->direita != NULL) {
            atual = atual->direita;
        } else if (escolha == 's') {
            printf("\nVocê decidiu encerrar a investigação.\n");
            break;
        } else {
            printf("Opção inválida! Tente novamente.\n");
        }
    }
}

/// ===============================================
/// FUNÇÕES DE LIMPEZA
/// ===============================================

// Libera a memória da árvore de salas
void liberarSalas(Sala *raiz) {
    if (raiz == NULL) return;
    liberarSalas(raiz->esquerda);
    liberarSalas(raiz->direita);
    free(raiz);
}

// Libera a memória da árvore de pistas
void liberarPistas(PistaNode *raiz) {
    if (raiz == NULL) return;
    liberarPistas(raiz->esquerda);
    liberarPistas(raiz->direita);
    free(raiz);
}

/// ===============================================
/// FUNÇÃO PRINCIPAL
/// ===============================================

int main() {
    // Mapa fixo da mansão
    Sala *hall = criarSala("Hall de Entrada", "Pegadas suspeitas no tapete");
    Sala *salaEstar = criarSala("Sala de Estar", "Um relógio parado às 3h15");
    Sala *cozinha = criarSala("Cozinha", "Um copo quebrado no chão");
    Sala *biblioteca = criarSala("Biblioteca", "Um livro faltando na estante");
    Sala *jardim = criarSala("Jardim", "");
    Sala *porao = criarSala("Porão", "Um cofre trancado");
    Sala *torre = criarSala("Torre", "Uma janela aberta recentemente");

    // Conexões da árvore (mapa da mansão)
    hall->esquerda = salaEstar;
    hall->direita = cozinha;
    salaEstar->esquerda = biblioteca;
    salaEstar->direita = jardim;
    cozinha->esquerda = porao;
    cozinha->direita = torre;

    PistaNode *arvorePistas = NULL;

    printf("=== DETECTIVE QUEST: O Mistério das Pistas ===\n");
    printf("Você inicia sua investigação no Hall de Entrada...\n");

    explorarSalasComPistas(hall, &arvorePistas);

    printf("\n==============================================\n");
    printf("🧾 PISTAS COLETADAS (em ordem alfabética):\n\n");
    if (arvorePistas != NULL)
        exibirPistas(arvorePistas);
    else
        printf("Nenhuma pista foi coletada!\n");
    printf("==============================================\n");

    liberarSalas(hall);
    liberarPistas(arvorePistas);
    printf("\nObrigado por jogar Detective Quest!\n");

    return 0;
}
