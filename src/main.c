#include "raylib.h"
#include "constantes.h"
#include "jogo.h"
#include "interface.h"
#define CAMINHO_MUSICA_PADRAO "assets/musica_fase1.ogg"

int TECLAS_LANE[QUANTIDADE_LANES] = {KEY_D, KEY_F, KEY_J, KEY_K};

int main(void){
    InitWindow(LARGURA_JANELA, ALTURA_JANELA, TITULO_JANELA);
    InitAudioDevice();
    SetTargetFPS(FPS_ALVO);

    Music musica = { 0 };
    bool tem_musica = FileExists(CAMINHO_MUSICA_PADRAO);
    bool musica_iniciada = false;

    tem_musica = false;
    
    if(tem_musica == true){
        musica = LoadMusicStream(CAMINHO_MUSICA_PADRAO);
        musica.looping = false;
    }
    
    EstadoJogo jogo;

    if(!iniciarJogo(&jogo, CAMINHO_MAPA_PADRAO)){
        CloseAudioDevice();
        CloseWindow();
        return 1;
    }

    else{
        while (!WindowShouldClose()){
            float delta_tempo = GetFrameTime();
            
            if(jogo.fase == FASE_JOGANDO){
                if(tem_musica ){   
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
                
                for(int i=0; i<QUANTIDADE_LANES; i++){
                    if(IsKeyPressed(TECLAS_LANE[i])){
                        tratarTeclaLane(&jogo, i);
                    }
                }
            }
            
            atualizarJogo(&jogo, delta_tempo);
            BeginDrawing();
            ClearBackground(BLACK);
            
            if(jogo.fase == FASE_VITORIA || jogo.fase == FASE_DERROTA){
                desenharTelaFinal(&jogo);
            }
            
            else{
                desenharJogo(&jogo);
            }
            
            EndDrawing();
        }
    }
        
    encerrarJogo(&jogo);

    if(tem_musica){
        UnloadMusicStream(musica);
    }

    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}