#ifndef MENU_H
#define MENU_H

typedef enum OpcaoMenu{
    MENU_JOGAR,
    MENU_COMO_JOGAR,
    MENU_VOLUME,
    MENU_SAIR,
    QUANTIDADE_OPCOES_MENU
} OpcaoMenu;

typedef enum Dificuldade{
    DIFICULDADE_FACIL,
    DIFICULDADE_MEDIO,
    DIFICULDADE_DIFICIL,
    QUANTIDADE_DIFICULDADES
} Dificuldade;

void carregarMenu(void);
void descarregarMenu(void);
void desenharMenu(OpcaoMenu opcao_selecionada);
int atualizarMenu(OpcaoMenu *opcao_selecionada);
void desenharComoJogar(void);
int atualizarSelecaoDificuldade(Dificuldade *dificuldade_selecionada);
void desenharSelecaoDificuldade(Dificuldade dificuldade_selecionada);

/* Tela temporaria de escolha da musica. A ultima opcao (indice quantidade_musicas) e "Adicionar musica".
   atualizarSelecaoMusica retorna 1 quando o jogador aperta ENTER. */
int atualizarSelecaoMusica(int *opcao_selecionada, int quantidade_musicas);
void desenharSelecaoMusica(char **caminhos_musicas, int quantidade_musicas, int opcao_selecionada, const char *mensagem);

/* Tela de volume: setas esquerda/direita mudam o volume em passos de PASSO_VOLUME.
   atualizarVolume retorna 1 quando o volume mudou neste frame. */
int atualizarVolume(int *volume_percentual);
void desenharVolume(int volume_percentual, const char *nome_previa);

/* Le o volume salvo em CAMINHO_CONFIGURACAO (ou VOLUME_PADRAO se nao houver) e salva o escolhido. */
int carregarVolume(void);
void salvarVolume(int volume_percentual);

#endif
