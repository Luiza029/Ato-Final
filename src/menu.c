#include <stdio.h>
#include "raylib.h"
#include "menu.h"
#include "constantes.h"
#include "constantes_raylib.h"
#include "musicas.h"

static const char *TEXTOS_OPCOES[QUANTIDADE_OPCOES_MENU] = {
    "Jogar",
    "Como jogar",
    "Volume",
    "Sair"
};

static const char *TEXTOS_DIFICULDADES[QUANTIDADE_DIFICULDADES] = {
    "Facil",
    "Medio",
    "Dificil"
};

static const char *DESCRICOES_DIFICULDADES[QUANTIDADE_DIFICULDADES] = {
    "Para aprender o ritmo.",
    "Para quem ja pegou o jeito.",
    "Para sobreviventes de verdade."
};

static const char *LINHAS_COMO_JOGAR[] = {
    "Aperte D, F, J e K quando a nota chegar na linha.",
    "Acertos afastam o assassino. Erros o aproximam.",
    "Errou demais? O assassino joga notas extras na pauta.",
    "ESPACO: rewind, volta algumas notas (usos limitados).",
    "Se o assassino te alcancar, fim de jogo."
};
#define QUANTIDADE_LINHAS_COMO_JOGAR (int)(sizeof(LINHAS_COMO_JOGAR) / sizeof(LINHAS_COMO_JOGAR[0]))

static Texture2D textura_fundo_menu;
static bool tem_fundo_menu = false;

static void desenharTextoCentralizado(const char *texto, int centro_x, int posicao_y, int tamanho, Color cor){
    int largura_texto = MeasureText(texto, tamanho);
    DrawText(texto, centro_x - largura_texto / 2, posicao_y, tamanho, cor);
}

void carregarMenu(void){
    if(FileExists(CAMINHO_FUNDO_MENU)){
        textura_fundo_menu = LoadTexture(CAMINHO_FUNDO_MENU);
        tem_fundo_menu = true;
    }
}

void descarregarMenu(void){
    if(tem_fundo_menu){
        UnloadTexture(textura_fundo_menu);
        tem_fundo_menu = false;
    }
}

static void desenharArteMenu(void){
    int centro_area_arte = LARGURA_COLUNA_MENU + (LARGURA_JANELA - LARGURA_COLUNA_MENU) / 2;

    if(!tem_fundo_menu){
        desenharTextoCentralizado(TITULO_JANELA, centro_area_arte, (ALTURA_JANELA - TAMANHO_FONTE_TITULO) / 2, TAMANHO_FONTE_TITULO, COR_TEXTO_DESTAQUE);
        return;
    }

    float escala = (float)ALTURA_JANELA / (float)textura_fundo_menu.height;
    if(escala >= 1.0f){
        escala = (float)(int)escala;
    }

    float largura_arte = textura_fundo_menu.width * escala;
    float altura_arte = textura_fundo_menu.height * escala;
    Vector2 posicao = { LARGURA_JANELA - largura_arte, (ALTURA_JANELA - altura_arte) / 2.0f };

    DrawTextureEx(textura_fundo_menu, posicao, 0.0f, escala, WHITE);
}

static int navegarOpcoes(int opcao, int quantidade_opcoes){
    if(IsKeyPressed(TECLA_MENU_BAIXO)){
        opcao = (opcao + 1) % quantidade_opcoes;
    }
    if(IsKeyPressed(TECLA_MENU_CIMA)){
        opcao = (opcao - 1 + quantidade_opcoes) % quantidade_opcoes;
    }

    return opcao;
}

int atualizarMenu(OpcaoMenu *opcao_selecionada){
    *opcao_selecionada = (OpcaoMenu)navegarOpcoes((int)*opcao_selecionada, QUANTIDADE_OPCOES_MENU);

    return IsKeyPressed(TECLA_CONFIRMAR);
}

int atualizarSelecaoDificuldade(Dificuldade *dificuldade_selecionada){
    *dificuldade_selecionada = (Dificuldade)navegarOpcoes((int)*dificuldade_selecionada, QUANTIDADE_DIFICULDADES);

    return IsKeyPressed(TECLA_CONFIRMAR);
}

void desenharMenu(OpcaoMenu opcao_selecionada){
    DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_JANELA, COR_FUNDO_MENU);
    desenharArteMenu();

    int altura_opcoes = QUANTIDADE_OPCOES_MENU * ESPACO_OPCOES_MENU;
    int posicao_y = (ALTURA_JANELA - altura_opcoes) / 2;
    int largura_marcador = MeasureText("> ", TAMANHO_FONTE_OPCOES_MENU);

    for(int opcao = 0; opcao < QUANTIDADE_OPCOES_MENU; opcao++){
        if(opcao == (int)opcao_selecionada){
            DrawText(">", POSICAO_X_OPCOES_MENU - largura_marcador, posicao_y, TAMANHO_FONTE_OPCOES_MENU, COR_TEXTO_DESTAQUE);
            DrawText(TEXTOS_OPCOES[opcao], POSICAO_X_OPCOES_MENU, posicao_y, TAMANHO_FONTE_OPCOES_MENU, COR_TEXTO_DESTAQUE);
        }else{
            DrawText(TEXTOS_OPCOES[opcao], POSICAO_X_OPCOES_MENU, posicao_y, TAMANHO_FONTE_OPCOES_MENU, Fade(COR_TEXTO, OPACIDADE_OPCAO_MENU));
        }
        posicao_y += ESPACO_OPCOES_MENU;
    }

    DrawText("Setas para escolher, ENTER para confirmar", MARGEM_TELA, ALTURA_JANELA - MARGEM_TELA - TAMANHO_FONTE_INFO, TAMANHO_FONTE_INFO, Fade(COR_TEXTO, OPACIDADE_OPCAO_MENU));
}

void desenharComoJogar(void){
    int centro_x = LARGURA_JANELA / 2;

    DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_JANELA, COR_FUNDO_MENU);

    desenharTextoCentralizado("Como jogar", centro_x, POSICAO_Y_TITULO_MENU, TAMANHO_FONTE_TITULO, COR_TEXTO_DESTAQUE);

    int posicao_y = POSICAO_Y_TITULO_MENU + TAMANHO_FONTE_TITULO + ESPACO_LINHAS_MENU;
    for(int linha = 0; linha < QUANTIDADE_LINHAS_COMO_JOGAR; linha++){
        desenharTextoCentralizado(LINHAS_COMO_JOGAR[linha], centro_x, posicao_y, TAMANHO_FONTE_TEXTO, COR_TEXTO);
        posicao_y += ESPACO_LINHAS_MENU;
    }

    desenharTextoCentralizado(TextFormat("Acerte pelo menos %.0f%% das notas para escapar.", PERCENTUAL_MINIMO_VITORIA * 100.0f), centro_x, posicao_y, TAMANHO_FONTE_TEXTO, COR_PERFEITO);

    desenharTextoCentralizado("ENTER para voltar", centro_x, ALTURA_JANELA - MARGEM_TELA - TAMANHO_FONTE_INFO, TAMANHO_FONTE_INFO, COR_TEXTO);
}


void desenharSelecaoDificuldade(Dificuldade dificuldade_selecionada){
    int centro_x = LARGURA_JANELA / 2;

    DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_JANELA, COR_FUNDO_MENU);

    desenharTextoCentralizado("Escolha a dificuldade", centro_x, POSICAO_Y_TITULO_MENU, TAMANHO_FONTE_TITULO, COR_TEXTO_DESTAQUE);

    int altura_opcoes = QUANTIDADE_DIFICULDADES * ESPACO_OPCOES_MENU;
    int posicao_y = (ALTURA_JANELA - altura_opcoes) / 2;

    for(int dificuldade = 0; dificuldade < QUANTIDADE_DIFICULDADES; dificuldade++){
        if(dificuldade == (int)dificuldade_selecionada){
            desenharTextoCentralizado(TextFormat("> %s <", TEXTOS_DIFICULDADES[dificuldade]), centro_x, posicao_y, TAMANHO_FONTE_OPCOES_MENU, COR_TEXTO_DESTAQUE);
        }else{
            desenharTextoCentralizado(TEXTOS_DIFICULDADES[dificuldade], centro_x, posicao_y,TAMANHO_FONTE_OPCOES_MENU, Fade(COR_TEXTO, OPACIDADE_OPCAO_MENU));
        }
        posicao_y += ESPACO_OPCOES_MENU;
    }

    desenharTextoCentralizado(DESCRICOES_DIFICULDADES[dificuldade_selecionada], centro_x, posicao_y + ESPACO_LINHAS_MENU,TAMANHO_FONTE_TEXTO, COR_TEXTO);

    desenharTextoCentralizado("Setas para escolher, ENTER para jogar, BACKSPACE para voltar", centro_x, ALTURA_JANELA - MARGEM_TELA - TAMANHO_FONTE_INFO, TAMANHO_FONTE_INFO, Fade(COR_TEXTO, OPACIDADE_OPCAO_MENU));
}

/* Tela temporaria de escolha da musica: as musicas da pasta e, por ultimo, o botao de adicionar. */
int atualizarSelecaoMusica(int *opcao_selecionada, int quantidade_musicas){
    *opcao_selecionada = navegarOpcoes(*opcao_selecionada, quantidade_musicas + 1);

    return IsKeyPressed(TECLA_CONFIRMAR);
}

void desenharSelecaoMusica(char **caminhos_musicas, int quantidade_musicas, int opcao_selecionada, const char *mensagem){
    int centro_x = LARGURA_JANELA / 2;
    int total_opcoes = quantidade_musicas + 1;

    DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_JANELA, COR_FUNDO_MENU);

    desenharTextoCentralizado("Escolha a musica", centro_x, POSICAO_Y_TITULO_MENU, TAMANHO_FONTE_TITULO, COR_TEXTO_DESTAQUE);

    // Mostra no maximo MAXIMO_MUSICAS_VISIVEIS linhas, rolando junto com a selecao
    int primeira_visivel = 0;
    if(opcao_selecionada >= MAXIMO_MUSICAS_VISIVEIS){
        primeira_visivel = opcao_selecionada - MAXIMO_MUSICAS_VISIVEIS + 1;
    }

    int posicao_y = POSICAO_Y_LISTA_MUSICAS;

    for(int opcao = primeira_visivel; opcao < total_opcoes && opcao < primeira_visivel + MAXIMO_MUSICAS_VISIVEIS; opcao++){
        char texto[TAMANHO_MAXIMO_NOME_MUSICA];

        if(opcao < quantidade_musicas){
            montarNomeExibicao(texto, TAMANHO_MAXIMO_NOME_MUSICA, caminhos_musicas[opcao]);
        }else{
            TextCopy(texto, "+ Adicionar musica");
        }

        if(opcao == opcao_selecionada){
            desenharTextoCentralizado(TextFormat("> %s <", texto), centro_x, posicao_y, TAMANHO_FONTE_TEXTO, COR_TEXTO_DESTAQUE);
        }else{
            desenharTextoCentralizado(texto, centro_x, posicao_y, TAMANHO_FONTE_TEXTO, Fade(COR_TEXTO, OPACIDADE_OPCAO_MENU));
        }
        posicao_y += ESPACO_LINHAS_MENU;
    }

    if(primeira_visivel + MAXIMO_MUSICAS_VISIVEIS < total_opcoes){
        desenharTextoCentralizado("...", centro_x, posicao_y, TAMANHO_FONTE_TEXTO, Fade(COR_TEXTO, OPACIDADE_OPCAO_MENU));
    }

    if(mensagem[0] != '\0'){
        desenharTextoCentralizado(mensagem, centro_x, ALTURA_JANELA - MARGEM_TELA - 3 * TAMANHO_FONTE_INFO - 2 * MARGEM_TELA, TAMANHO_FONTE_INFO, COR_PERFEITO);
    }

    desenharTextoCentralizado("Ou arraste um arquivo .mp3, .ogg ou .wav para a janela", centro_x, ALTURA_JANELA - MARGEM_TELA - 2 * TAMANHO_FONTE_INFO - MARGEM_TELA, TAMANHO_FONTE_INFO, COR_TEXTO);

    desenharTextoCentralizado("Setas para escolher, ENTER para confirmar, BACKSPACE para voltar", centro_x, ALTURA_JANELA - MARGEM_TELA - TAMANHO_FONTE_INFO, TAMANHO_FONTE_INFO, Fade(COR_TEXTO, OPACIDADE_OPCAO_MENU));
}

/* Tela de volume: o volume vai de 0 a VOLUME_MAXIMO, em passos de PASSO_VOLUME. */
int atualizarVolume(int *volume_percentual){
    int volume_anterior = *volume_percentual;

    if(IsKeyPressed(TECLA_MENU_DIREITA) && *volume_percentual < VOLUME_MAXIMO){
        *volume_percentual += PASSO_VOLUME;
    }
    if(IsKeyPressed(TECLA_MENU_ESQUERDA) && *volume_percentual > 0){
        *volume_percentual -= PASSO_VOLUME;
    }

    return *volume_percentual != volume_anterior;
}

void desenharVolume(int volume_percentual, const char *nome_previa){
    int centro_x = LARGURA_JANELA / 2;

    DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_JANELA, COR_FUNDO_MENU);

    desenharTextoCentralizado("Volume", centro_x, POSICAO_Y_TITULO_MENU, TAMANHO_FONTE_TITULO, COR_TEXTO_DESTAQUE);

    // Barra: contorno com o tamanho total e o preenchimento proporcional ao volume
    int barra_x = centro_x - LARGURA_BARRA_VOLUME / 2;
    int largura_preenchida = LARGURA_BARRA_VOLUME * volume_percentual / VOLUME_MAXIMO;

    DrawRectangle(barra_x, POSICAO_Y_BARRA_VOLUME, largura_preenchida, ALTURA_BARRA_VOLUME, COR_TEXTO_DESTAQUE);
    DrawRectangleLines(barra_x, POSICAO_Y_BARRA_VOLUME, LARGURA_BARRA_VOLUME, ALTURA_BARRA_VOLUME, COR_TEXTO);

    desenharTextoCentralizado(TextFormat("< %d%% >", volume_percentual), centro_x, POSICAO_Y_BARRA_VOLUME + ALTURA_BARRA_VOLUME + ESPACO_LINHAS_MENU / 2, TAMANHO_FONTE_OPCOES_MENU, COR_TEXTO);

    if(nome_previa[0] != '\0'){
        desenharTextoCentralizado(TextFormat("Tocando: %s", nome_previa), centro_x, POSICAO_Y_BARRA_VOLUME + ALTURA_BARRA_VOLUME + 2 * ESPACO_OPCOES_MENU, TAMANHO_FONTE_INFO, COR_PERFEITO);
    }

    desenharTextoCentralizado("Setas esquerda e direita para ajustar, ENTER ou BACKSPACE para voltar", centro_x, ALTURA_JANELA - MARGEM_TELA - TAMANHO_FONTE_INFO, TAMANHO_FONTE_INFO, Fade(COR_TEXTO, OPACIDADE_OPCAO_MENU));
}

int carregarVolume(void){
    FILE *arq = fopen(CAMINHO_CONFIGURACAO, "r");
    int volume = VOLUME_PADRAO;

    if(arq != NULL){
        if(fscanf(arq, "volume %d", &volume) != 1 || volume < 0 || volume > VOLUME_MAXIMO){
            volume = VOLUME_PADRAO;
        }
        fclose(arq);
    }

    return volume;
}

void salvarVolume(int volume_percentual){
    FILE *arq = fopen(CAMINHO_CONFIGURACAO, "w");

    if(arq != NULL){
        fprintf(arq, "volume %d\n", volume_percentual);
        fclose(arq);
    }
}
