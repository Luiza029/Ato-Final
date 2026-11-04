#include "raylib.h"
#include "constantes.h"
#include "constantes_raylib.h"
#include "jogo.h"
#include "interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "menu.h"
#include "gerador.h"
#include "musicas.h"
#include "arquivos_windows.h"

typedef enum Tela{
    TELA_MENU,
    TELA_COMO_JOGAR,
    TELA_MUSICA,
    TELA_DIFICULDADE,
    TELA_JOGO
} Tela;

/* Troca a musica da partida: prepara os mapas dela (gerando se precisar) e carrega o audio.
   Se der errado, mantem a musica e os mapas que estavam, escreve o aviso em mensagem e retorna 0. */
static int trocarMusica(const char *caminho_musica, Music *musica, bool *tem_musica,
                        char caminhos_mapas[QUANTIDADE_DIFICULDADES][TAMANHO_MAXIMO_CAMINHO], char *mensagem){
    if(!prepararMapasDaMusica(caminho_musica, caminhos_mapas)){
        snprintf(mensagem, TAMANHO_MENSAGEM_MUSICA, "Nao foi possivel abrir %s", GetFileName(caminho_musica));
        return 0;
    }

    if(*tem_musica){
        UnloadMusicStream(*musica);
    }

    *musica = LoadMusicStream(caminho_musica);
    *tem_musica = IsMusicValid(*musica);
    musica->looping = false;

    return 1;
}

/* Copia a musica nova para a pasta do jogo, gera os mapas e recarrega a lista.
   Se der certo, deixa a musica nova selecionada na tela. */
static void processarMusicaNova(const char *caminho_origem, FilePathList *musicas, int *opcao_musica, char *mensagem){
    char caminho_copia[TAMANHO_MAXIMO_CAMINHO];

    if(!adicionarMusica(caminho_origem, caminho_copia, TAMANHO_MAXIMO_CAMINHO)){
        snprintf(mensagem, TAMANHO_MENSAGEM_MUSICA, "Nao foi possivel ler %s. Use .mp3, .ogg ou .wav", GetFileName(caminho_origem));
        return;
    }

    descarregarListaMusicas(*musicas);
    *musicas = carregarListaMusicas();

    for(unsigned int i = 0; i < musicas->count; i++){
        if(TextIsEqual(GetFileName(musicas->paths[i]), GetFileName(caminho_copia))){
            *opcao_musica = (int)i;
        }
    }

    char nome[TAMANHO_MAXIMO_NOME_MUSICA];
    montarNomeExibicao(nome, TAMANHO_MAXIMO_NOME_MUSICA, caminho_copia);
    snprintf(mensagem, TAMANHO_MENSAGEM_MUSICA, "Musica adicionada: %s", nome);
}

int main(void){
    InitWindow(LARGURA_JANELA, ALTURA_JANELA, TITULO_JANELA);
    InitAudioDevice();
    SetTargetFPS(FPS_ALVO);
    srand((unsigned int)time(NULL));
    carregarMenu();

    FilePathList musicas = carregarListaMusicas();
    int opcao_musica = 0;
    char mensagem_musica[TAMANHO_MENSAGEM_MUSICA] = "";

    // Musica escolhida (arrastada ou pelo botao) que sera copiada e analisada no proximo frame,
    // depois de a tela mostrar o aviso de "aguarde"
    char musica_pendente[TAMANHO_MAXIMO_CAMINHO] = "";

    char caminhos_mapas[QUANTIDADE_DIFICULDADES][TAMANHO_MAXIMO_CAMINHO];

    Music musica = { 0 };
    bool tem_musica = false;
    bool musica_iniciada = false;
    bool musica_pausada = false;

    EstadoJogo jogo;
    Tela tela_atual = TELA_MENU;
    OpcaoMenu opcao_menu = MENU_JOGAR;
    Dificuldade dificuldade = DIFICULDADE_FACIL;
    bool partida_ativa = false;
    bool rodando = true;

    while(rodando && !WindowShouldClose()){
        float delta_tempo = GetFrameTime();

        if(musica_pendente[0] != '\0'){
            processarMusicaNova(musica_pendente, &musicas, &opcao_musica, mensagem_musica);
            musica_pendente[0] = '\0';
        }

        // Musica arrastada para a janela (fora da partida): vai para a tela de musicas e e adicionada
        if(IsFileDropped() && tela_atual != TELA_JOGO){
            FilePathList arquivos = LoadDroppedFiles();

            if(arquivos.count > 0){
                snprintf(musica_pendente, TAMANHO_MAXIMO_CAMINHO, "%s", arquivos.paths[0]);
                snprintf(mensagem_musica, TAMANHO_MENSAGEM_MUSICA, "Gerando os mapas de %s, aguarde...", GetFileName(musica_pendente));
                tela_atual = TELA_MUSICA;
            }

            UnloadDroppedFiles(arquivos);
        }

        if(tela_atual == TELA_MENU){
            if(atualizarMenu(&opcao_menu)){
                if(opcao_menu == MENU_JOGAR){
                    mensagem_musica[0] = '\0';
                    tela_atual = TELA_MUSICA;
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

        else if(tela_atual == TELA_MUSICA){
            int quantidade_musicas = (int)musicas.count;

            if(atualizarSelecaoMusica(&opcao_musica, quantidade_musicas)){
                // Ultima opcao: botao "Adicionar musica"
                if(opcao_musica == quantidade_musicas){
                    char escolhida[TAMANHO_MAXIMO_CAMINHO];

                    if(escolherArquivoMusica(GetWindowHandle(), escolhida, TAMANHO_MAXIMO_CAMINHO)){
                        snprintf(musica_pendente, TAMANHO_MAXIMO_CAMINHO, "%s", escolhida);
                        snprintf(mensagem_musica, TAMANHO_MENSAGEM_MUSICA, "Gerando os mapas de %s, aguarde...", GetFileName(musica_pendente));
                    }
                }

                else if(trocarMusica(musicas.paths[opcao_musica], &musica, &tem_musica, caminhos_mapas, mensagem_musica)){
                    tela_atual = TELA_DIFICULDADE;
                }
            }

            else if(IsKeyPressed(TECLA_VOLTAR)){
                tela_atual = TELA_MENU;
            }
        }

        else if(tela_atual == TELA_DIFICULDADE){
            if(atualizarSelecaoDificuldade(&dificuldade)){
                if(iniciarJogo(&jogo, caminhos_mapas[dificuldade])){
                    partida_ativa = true;
                    musica_iniciada = false;
                    musica_pausada = false;
                    tela_atual = TELA_JOGO;
                }
            }

            else if(IsKeyPressed(TECLA_VOLTAR)){
                tela_atual = TELA_MUSICA;
            }
        }

        else{
            if(jogo.fase == FASE_JOGANDO){
                // Depois de um rewind para antes do inicio da musica, o tempo anda sozinho ate 0
                if(tem_musica && jogo.tempo_atual >= 0.0f){
                    if(!musica_iniciada){
                        PlayMusicStream(musica);
                        musica_iniciada = true;
                    }

                    else if(musica_pausada){
                        // Resume continua a musica pausada. PlayMusicStream nao serve aqui:
                        // ele recomeca a musica do inicio e desfaz o SeekMusicStream.
                        SeekMusicStream(musica, jogo.tempo_atual);
                        ResumeMusicStream(musica);

                        // Se a musica ja tinha acabado (rewind na espera do fim), ela precisa tocar de novo
                        if(!IsMusicStreamPlaying(musica)){
                            PlayMusicStream(musica);
                            SeekMusicStream(musica, jogo.tempo_atual);
                        }

                        musica_pausada = false;
                    }

                    UpdateMusicStream(musica);
                }

                // Enquanto a musica toca, o relogio do jogo e o da musica. Quando ela acaba, segue pelo frame.
                if(tem_musica && IsMusicStreamPlaying(musica)){
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

                    // A musica para durante a animacao e volta do ponto novo depois
                    if(tem_musica && musica_iniciada && jogo.fase == FASE_REWIND){
                        PauseMusicStream(musica);
                        musica_pausada = true;
                    }
                }
            }

            atualizarJogo(&jogo, delta_tempo);

            // Fim da partida: para a musica (sem UpdateMusicStream ela ficaria repetindo um pedaco)
            if(tem_musica && (jogo.fase == FASE_VITORIA || jogo.fase == FASE_DERROTA) && IsMusicStreamPlaying(musica)){
                StopMusicStream(musica);
            }

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

        else if(tela_atual == TELA_MUSICA){
            desenharSelecaoMusica(musicas.paths, (int)musicas.count, opcao_musica, mensagem_musica);
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

    descarregarListaMusicas(musicas);
    descarregarMenu();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
