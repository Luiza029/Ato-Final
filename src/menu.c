#include "raylib.h"
#include "menu.h"
#include "constantes.h"
#include "constantes_raylib.h"

static const char *TEXTOS_OPCOES[QUANTIDADE_OPCOES_MENU] = {
    "Jogar",
    "Como jogar",
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

    /* Aviso temporário: enquanto os mapas definitivos não ficam prontos */
    desenharTextoCentralizado("Por enquanto, as tres dificuldades usam o mapa de teste.", centro_x, ALTURA_JANELA - MARGEM_TELA - 2 * TAMANHO_FONTE_INFO - MARGEM_TELA, TAMANHO_FONTE_INFO, COR_PERFEITO);

    desenharTextoCentralizado("Setas para escolher, ENTER para jogar, BACKSPACE para voltar", centro_x, ALTURA_JANELA - MARGEM_TELA - TAMANHO_FONTE_INFO, TAMANHO_FONTE_INFO, Fade(COR_TEXTO, OPACIDADE_OPCAO_MENU));
}
