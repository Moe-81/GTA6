#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <conio.h>
// o nomes das estruturas-variaveis estão bem alto explicativas
#define MAX_ID_LENGTH 20
#define LARGURA 50
#define ALTURA 20
#define ARQUIVO_ESTATS "estatisticas.bin" 

// Estruturas
typedef struct {
    char nome[MAX_ID_LENGTH];
    int pontos;
    int fase;
} Jogador;

typedef struct {
    int x, y, dx, dy;
} Bola;

typedef struct {
    int x, y;
} Raquete;

typedef struct {
    int vitorias;
    int derrotas;
    int pontosMaximos;
} Estatisticas;

// Funções 
void limparTela() {
#ifdef _WIN32
    system("cls"); //para ser compativel com linux e windows 
#else
    system("clear");
#endif
}

void desenharParedes() {
    int i, j;
    for (i = 0; i < ALTURA; i++) {
        for (j = 0; j < LARGURA; j++) {
            if (i == 0 || i == ALTURA - 1)
                printf("=");
            else if (j == 0 || j == LARGURA - 1)
                printf("|");
            else
                printf(" ");
        }
        printf("\n");
    }
}

void desenharRaquete(Raquete *raquete) {
    int i;
    for (i = 0; i < 4; i++) {
        printf("\033[%d;%dH", raquete->y + i, raquete->x);
        putchar('|');
    }
}

void desenharBola(Bola *bola) {
    printf("\033[%d;%dH", bola->y, bola->x);
    putchar('O');
}

void moverBola(Bola *bola) {
    bola->x += bola->dx;
    bola->y += bola->dy;

    if (bola->y == 1 || bola->y == ALTURA - 2)
        bola->dy = -bola->dy;

    if (bola->x == LARGURA - 1)
        bola->dx = -bola->dx;
}

void exibirGameOver(Jogador *jogador, Estatisticas *estatisticas) {
    limparTela();
    printf("############## GAME OVER ##############\n");
    printf("Jogador: %s - Pontos: %d\n", jogador->nome, jogador->pontos);
    printf("Pressione '9' para voltar ao menu principal.\n");

    // Atualiza as estatisticas
    if (jogador->pontos > estatisticas->pontosMaximos) {
        estatisticas->pontosMaximos = jogador->pontos; 
    }
    estatisticas->derrotas++;

    // Salva as estatisticas 
    FILE *arquivo = fopen(ARQUIVO_ESTATS, "wb");
    if (arquivo != NULL) {
        fwrite(estatisticas, sizeof(Estatisticas), 1, arquivo);
        fclose(arquivo);
    }

    char tecla;
    do {
        tecla = getch(); // Espera o jogador pressionar uma tecla
    } while (tecla != '9'); // Continua até que '9' seja pressionado
}

void movimentarRaquete(Raquete *raquete, char tecla) {
    if (tecla == 'w' && raquete->y > 1)
        raquete->y--;
    if (tecla == 's' && raquete->y < ALTURA - 5)
        raquete->y++;
}

void carregarConfiguracaoFase(int fase, int *velocidadeBola, int *pontosProximaFase, int *posRaquete) {
    FILE *arquivo;
    char nomeArquivo[15];
    sprintf(nomeArquivo, "fase%d.txt", fase + 1);

    arquivo = fopen(nomeArquivo, "r");
    if (arquivo == NULL) {
        exit(1);
    }

    fscanf(arquivo, "%d %d %d", velocidadeBola, pontosProximaFase, posRaquete);
    fclose(arquivo);
}

void carregarEstatisticas(Estatisticas *estatisticas) {
    FILE *arquivo = fopen(ARQUIVO_ESTATS, "rb");
    if (arquivo != NULL) {
        fread(estatisticas, sizeof(Estatisticas), 1, arquivo);
        fclose(arquivo);
    } else {
        estatisticas->vitorias = 0;
        estatisticas->derrotas = 0;
        estatisticas->pontosMaximos = 0;
    }
}

void iniciarJogo(Jogador *jogador, Estatisticas *estatisticas) {
    Bola *bola = malloc(sizeof(Bola));
    Raquete *raquete = malloc(sizeof(Raquete));
    int velocidadeBola, pontosProximaFase, posRaquete;
    char tecla;
    int pontosJogador = jogador->pontos;

    int fase = jogador->fase;

    while (1) {
        carregarConfiguracaoFase(fase, &velocidadeBola, &pontosProximaFase, &posRaquete);

        bola->x = LARGURA / 2;
        bola->y = ALTURA / 2;
        bola->dx = 1;
        bola->dy = 1;

        raquete->x = 2;
        raquete->y = posRaquete;

        while (1) {
            limparTela();
            desenharParedes();
            desenharRaquete(raquete);
            desenharBola(bola);

            moverBola(bola);

            if (bola->x == raquete->x + 1 && bola->y >= raquete->y && bola->y <= raquete->y + 3) {
                bola->dx = -bola->dx;
                pontosJogador++;
            }

            if (bola->x <= 0) {
                jogador->pontos = pontosJogador;
                exibirGameOver(jogador, estatisticas);
                free(bola);
                free(raquete);
                return;
            }

            printf("\033[%d;0H", ALTURA);
            printf("Jogador: %s | Pontos: %d | Fase: %d\n", jogador->nome, pontosJogador, fase + 1);

            if (_kbhit()) {
                tecla = getch();
                movimentarRaquete(raquete, tecla);
            }

            if (pontosJogador >= pontosProximaFase) {
                fase++;
                jogador->fase = fase;
                break;
            }

            usleep(50000 / velocidadeBola);
        }
    }
}

void exibirMenu(Jogador *jogador, Estatisticas *estatisticas) {
    int opcao;
    while (1) {
        limparTela();
        printf("#############################\n");
        printf("###       Pong Menu       ###\n");
        printf("#############################\n");
        printf("1 - Iniciar Jogo\n");
        printf("2 - Exibir Estatisticas\n");
        printf("3 - Fechar Jogo\n");
        printf("Escolha uma opção: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                printf("Digite seu ID de jogador: ");
                scanf("%s", jogador->nome);
                jogador->pontos = 0;
                jogador->fase = 0;
                iniciarJogo(jogador, estatisticas);
                break;

            case 2:
                printf("\nEstatísticas do Jogo:\n");
                printf("Vitorias: %d\n", estatisticas->vitorias);
                printf("Derrotas: %d\n", estatisticas->derrotas);
                printf("Maior Pontuação: %d\n", estatisticas->pontosMaximos);
                printf("\nPressione qualquer tecla para voltar ao menu.\n");
                getch();
                break;

            case 3:
                exit(0);

            default:
                break;
        }
    }
}

int main() {
    Jogador jogador;
    Estatisticas estatisticas;
    
    carregarEstatisticas(&estatisticas);
    exibirMenu(&jogador, &estatisticas);
    
    return 0;
}

