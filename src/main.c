#include "raylib.h"
#include "constantes.h"
#include "constantes_raylib.h"
#include "jogo.h"
#include "interface.h"
#include "menu.h"

typedef enum Tela{
    TELA_MENU,
    TELA_COMO_JOGAR,
    TELA_DIFICULDADE,
    TELA_JOGO
} Tela;

/* Mapa de cada dificuldade, na mesma ordem do enum Dificuldade */
static const char *CAMINHOS_MAPAS[QUANTIDADE_DIFICULDADES] = {
    CAMINHO_MAPA_FACIL,
    CAMINHO_MAPA_MEDIO,
    CAMINHO_MAPA_DIFICIL
};

int main(void){
    InitWindow(LARGURA_JANELA, ALTURA_JANELA, TITULO_JANELA);
    InitAudioDevice();
    SetTargetFPS(FPS_ALVO);
    carregarMenu();

    Music musica = { 0 };
    bool tem_musica = FileExists(CAMINHO_MUSICA_PADRAO);
    bool musica_iniciada = false;

    if(tem_musica){
        musica = LoadMusicStream(CAMINHO_MUSICA_PADRAO);
        musica.looping = false;
    }

    EstadoJogo jogo;
    Tela tela_atual = TELA_MENU;
    OpcaoMenu opcao_menu = MENU_JOGAR;
    Dificuldade dificuldade = DIFICULDADE_FACIL;
    bool partida_ativa = false;   
    bool rodando = true;          

    while(rodando && !WindowShouldClose()){
        float delta_tempo = GetFrameTime();

        if(tela_atual == TELA_MENU){
            if(atualizarMenu(&opcao_menu)){
                if(opcao_menu == MENU_JOGAR){
                    tela_atual = TELA_DIFICULDADE;
                }
                else if(opcao_menu == MENU_COMO_JOGAR){
                    tela_atual = TELA_COMO_JOGAR;
                }
                else{
                    rodando = false;
                }
            }
        }

        else if(tela_atual == TELA_COMO_JOGAR){
            if(IsKeyPressed(TECLA_CONFIRMAR)){
                tela_atual = TELA_MENU;
            }
        }

        else if(tela_atual == TELA_DIFICULDADE){
            if(atualizarSelecaoDificuldade(&dificuldade)){
                if(iniciarJogo(&jogo, CAMINHOS_MAPAS[dificuldade])){
                    partida_ativa = true;
                    musica_iniciada = false;
                    tela_atual = TELA_JOGO;
                }
            }

            else if(IsKeyPressed(TECLA_VOLTAR)){
                tela_atual = TELA_MENU;
            }
        }

        else{
            if(jogo.fase == FASE_JOGANDO){
                if(tem_musica){
                    if(!musica_iniciada){
                        PlayMusicStream(musica);
                        musica_iniciada = true;
                    }
                    UpdateMusicStream(musica);
                    jogo.tempo_atual = GetMusicTimePlayed(musica);
                }

                else{
                    jogo.tempo_atual += delta_tempo;
                }

                for(int i = 0; i < QUANTIDADE_LANES; i++){
                    if(IsKeyPressed(TECLAS_LANES[i])){
                        tratarTeclaLane(&jogo, i);
                    }
                }

                if(IsKeyPressed(TECLA_REWIND)){
                    usarRewind(&jogo);
                }
            }

            atualizarJogo(&jogo, delta_tempo);

            if((jogo.fase == FASE_VITORIA || jogo.fase == FASE_DERROTA) && IsKeyPressed(TECLA_CONFIRMAR)){
                encerrarJogo(&jogo);
                partida_ativa = false;
                tela_atual = TELA_MENU;

                if(tem_musica){
                    StopMusicStream(musica);
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if(tela_atual == TELA_MENU){
            desenharMenu(opcao_menu);
        }

        else if(tela_atual == TELA_COMO_JOGAR){
            desenharComoJogar();
        }

        else if(tela_atual == TELA_DIFICULDADE){
            desenharSelecaoDificuldade(dificuldade);
        }

        else if(jogo.fase == FASE_VITORIA || jogo.fase == FASE_DERROTA){
            desenharTelaFinal(&jogo);
        }

        else{
            desenharJogo(&jogo);
        }

        EndDrawing();
    }

    if(partida_ativa){
        encerrarJogo(&jogo);
    }

    if(tem_musica){
        UnloadMusicStream(musica);
    }

    descarregarMenu();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}