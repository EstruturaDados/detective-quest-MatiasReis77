#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estrutura que representa um cômodo da mansão
typedef struct Sala {
    char nome[50];
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// Cria dinamicamente uma nova sala com o nome informado
Sala* criarSala(const char *nome) {
    Sala *nova = (Sala*) malloc(sizeof(Sala));
    if (nova == NULL) {
        printf("Erro ao alocar memória para a sala.\n");
        exit(1);
    }
    strcpy(nova->nome, nome);
    nova->esquerda = NULL;
    nova->direita = NULL;
    return nova;
}

// Permite a exploração da mansão a partir de uma sala específica
void explorarSalas(Sala *atual) {
    char escolha;

    while (atual != NULL) {
        printf("\nVocê está em: %s\n", atual->nome);

        // Caso não existam caminhos à esquerda nem à direita, termina a exploração
        if (atual->esquerda == NULL && atual->direita == NULL) {
            printf("Fim da exploração! Você chegou ao fim do caminho.\n");
            break;
        }

        // Exibe opções disponíveis ao jogador
        printf("Escolha um caminho:\n");
        if (atual->esquerda != NULL)
            printf(" (e) Ir para %s à esquerda\n", atual->esquerda->nome);
        if (atual->direita != NULL)
            printf(" (d) Ir para %s à direita\n", atual->direita->nome);
        printf(" (s) Sair do jogo\n");
        printf("Sua escolha: ");
        scanf(" %c", &escolha);

        if (escolha == 'e' && atual->esquerda != NULL) {
            atual = atual->esquerda;
        } else if (escolha == 'd' && atual->direita != NULL) {
            atual = atual->direita;
        } else if (escolha == 's') {
            printf("Você decidiu encerrar a exploração.\n");
            break;
        } else {
            printf("Opção inválida! Tente novamente.\n");
        }
    }
}

// Libera toda a memória alocada da árvore
void liberarSalas(Sala *raiz) {
    if (raiz == NULL) return;
    liberarSalas(raiz->esquerda);
    liberarSalas(raiz->direita);
    free(raiz);
}

int main() {
    // Montagem fixa da mansão (árvore binária)
    Sala *hall = criarSala("Hall de Entrada");
    Sala *salaEstar = criarSala("Sala de Estar");
    Sala *cozinha = criarSala("Cozinha");
    Sala *biblioteca = criarSala("Biblioteca");
    Sala *jardim = criarSala("Jardim");
    Sala *porão = criarSala("Porão");
    Sala *torre = criarSala("Torre");

    // Conectando as salas (estrutura binária)
    hall->esquerda = salaEstar;
    hall->direita = cozinha;
    salaEstar->esquerda = biblioteca;
    salaEstar->direita = jardim;
    cozinha->esquerda = porão;
    cozinha->direita = torre;

    printf("=== Detective Quest: Exploração da Mansão ===\n");
    explorarSalas(hall);

    liberarSalas(hall); // Libera memória ao final
    printf("\nObrigado por jogar Detective Quest!\n");
    return 0;
}
    return 0;
}

