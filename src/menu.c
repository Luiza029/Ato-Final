#include "raylib.h"
#include "menu.h"
#include "constantes.h"
#include "constantes_raylib.h"

static const char *TEXTOS_OPCOES[QUANTIDADE_OPCOES_MENU] = {
    "Jogar",
    "Como jogar",
    "Sair"
};

static const char *LINHAS_COMO_JOGAR[] = {
    "Aperte D, F, J e K quando a nota chegar na linha.",
    "Acertos afastam o assassino. Erros o aproximam.",
    "Errou demais? O assassino joga notas extras na pauta.",
    "R: rewind, volta algumas notas (usos limitados).",
    "Se o assassino te alcancar, fim de jogo."
};
#define QUANTIDADE_LINHAS_COMO_JOGAR (int)(sizeof(LINHAS_COMO_JOGAR) / sizeof(LINHAS_COMO_JOGAR[0]))

static void desenharTextoCentralizado(const char *texto, int posicao_y, int tamanho, Color cor){
    int largura_texto = MeasureText(texto, tamanho);
    DrawText(texto, (LARGURA_JANELA - largura_texto) / 2, posicao_y, tamanho, cor);
}

int atualizarMenu(OpcaoMenu *opcao_selecionada){
    int opcao = (int)*opcao_selecionada;

    if(IsKeyPressed(KEY_DOWN)){
        opcao = (opcao + 1) % QUANTIDADE_OPCOES_MENU;
    }
    if(IsKeyPressed(KEY_UP)){
        opcao = (opcao - 1 + QUANTIDADE_OPCOES_MENU) % QUANTIDADE_OPCOES_MENU;
    }

    *opcao_selecionada = (OpcaoMenu)opcao;

    return IsKeyPressed(KEY_ENTER);
}

void desenharMenu(OpcaoMenu opcao_selecionada){
    DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_JANELA, COR_FUNDO_CENA);

    desenharTextoCentralizado(TITULO_JANELA, POSICAO_Y_TITULO_MENU, TAMANHO_FONTE_TITULO, COR_TEXTO_DESTAQUE);

    for(int opcao = 0; opcao < QUANTIDADE_OPCOES_MENU; opcao++){
        int posicao_y = POSICAO_Y_OPCOES_MENU + opcao * ESPACO_LINHAS_MENU;

        if(opcao == (int)opcao_selecionada){
            desenharTextoCentralizado(TextFormat("> %s <", TEXTOS_OPCOES[opcao]), posicao_y, TAMANHO_FONTE_TEXTO, COR_PERFEITO);
        }else{
            desenharTextoCentralizado(TEXTOS_OPCOES[opcao], posicao_y, TAMANHO_FONTE_TEXTO, COR_TEXTO);
        }
    }

    desenharTextoCentralizado("Setas para escolher, ENTER para confirmar",
                              ALTURA_JANELA - MARGEM_TELA - TAMANHO_FONTE_INFO, TAMANHO_FONTE_INFO, COR_TEXTO);
}

void desenharComoJogar(void){
    DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_JANELA, COR_FUNDO_CENA);

    desenharTextoCentralizado("Como jogar", POSICAO_Y_TITULO_MENU, TAMANHO_FONTE_TITULO, COR_TEXTO_DESTAQUE);

    int posicao_y = POSICAO_Y_TITULO_MENU + TAMANHO_FONTE_TITULO + ESPACO_LINHAS_MENU;
    for(int linha = 0; linha < QUANTIDADE_LINHAS_COMO_JOGAR; linha++){
        desenharTextoCentralizado(LINHAS_COMO_JOGAR[linha], posicao_y, TAMANHO_FONTE_TEXTO, COR_TEXTO);
        posicao_y += ESPACO_LINHAS_MENU;
    }

    desenharTextoCentralizado(TextFormat("Acerte pelo menos %.0f%% das notas para escapar.", PERCENTUAL_MINIMO_VITORIA * 100.0f),
                              posicao_y, TAMANHO_FONTE_TEXTO, COR_PERFEITO);

    desenharTextoCentralizado("ENTER para voltar",
                              ALTURA_JANELA - MARGEM_TELA - TAMANHO_FONTE_INFO, TAMANHO_FONTE_INFO, COR_TEXTO);
}