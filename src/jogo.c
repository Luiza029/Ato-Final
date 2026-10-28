#include <stdio.h>
#include <math.h>
#include "jogo.h"
#include "mapa.h"
#include "constantes.h"

int iniciarJogo(EstadoJogo *jogo, const char *caminho_mapa){
    criarLista(&jogo->lista);

    if(carregarMapa(&jogo->lista, caminho_mapa) == 0){
        return 0;
    }

    jogo->pontuacao = 0;
    jogo->combo = 0;
    jogo->erros_acumulados = 0;
    jogo->acertos_seguidos = 0;

    jogo->multiplicador = 1;
    jogo->distancia_assassino = DISTANCIA_INICIAL_ASSASSINO;
    jogo->rewinds_restantes = REWINDS_MAXIMOS;
    jogo->tempo_atual = jogo->tempo_atual - SILENCIO_INICIAL;

    jogo->fase = FASE_SILENCIO_INICIAL;
    jogo->temporizador_fase = 0;

    jogo->ultimo_julgamento = PENDENTE;
    jogo->tempo_feedback = 0;

    return 1;
}

void encerrarJogo(EstadoJogo *jogo){
    liberarLista(&jogo->lista);
}

static void registrarAcerto(EstadoJogo *jogo, EstadoNota resultado){
    jogo->combo++;
    jogo->multiplicador = 1 + jogo->combo / COMBO_POR_MULTIPLICADOR;
    
    if(jogo->multiplicador > MULTIPLICADOR_MAXIMO){
        jogo->multiplicador = MULTIPLICADOR_MAXIMO;
    }

    if(resultado == PERFEITO){
         jogo->pontuacao += PONTOS_PERFEITO * jogo->multiplicador;
    }

    else{
         jogo->pontuacao += PONTOS_BOM * jogo->multiplicador;
    }

    jogo->acertos_seguidos++;

    if(jogo->acertos_seguidos == ACERTOS_PARA_RECUPERAR){
        jogo->distancia_assassino += DISTANCIA_GANHA_POR_RECUPERACAO;

        if(jogo->distancia_assassino > DISTANCIA_MAXIMA_ASSASSINO){
            jogo->distancia_assassino = DISTANCIA_MAXIMA_ASSASSINO;
        }

        jogo->acertos_seguidos = 0;
    } 

    jogo->ultimo_julgamento = resultado;
    jogo->tempo_feedback = DURACAO_FEEDBACK;
}

static void registrarErro(EstadoJogo *jogo, int nota_era_injetada){
    jogo->combo = 0;
    jogo->acertos_seguidos = 0;
    jogo->multiplicador = 1;

    jogo->distancia_assassino -= DISTANCIA_PERDIDA_POR_ERRO;

    if(jogo->distancia_assassino < DISTANCIA_MINIMA_ASSASSINO){
        jogo->distancia_assassino = DISTANCIA_MINIMA_ASSASSINO;
    }

    if(!nota_era_injetada){
        jogo->erros_acumulados++;
    }

    jogo->ultimo_julgamento = ERRADO;
    jogo->tempo_feedback = DURACAO_FEEDBACK;
}

void tratarTeclaLane(EstadoJogo *jogo, int lane){
    if(!(jogo->fase == FASE_JOGANDO)){
        return;
    }

    Nota *nota = buscarNota(&jogo->lista, lane, jogo->tempo_atual);

    if(nota == NULL){
        registrarErro(jogo, 0);
        return;
    }

    float diferenca = fabsf(nota->tempo - jogo->tempo_atual);
    EstadoNota resultado;

    if(diferenca <= JANELA_PERFEITO){
        resultado = PERFEITO;
    } 

    else{
        resultado = BOM;
    }

    atualizarEstadoNota(nota, resultado);

    registrarAcerto(jogo, resultado);
}

void atualizarJogo(EstadoJogo *jogo, float delta_tempo){
    if(jogo->tempo_feedback > 0){
        jogo->tempo_feedback -= delta_tempo;
    }

    if(jogo->fase == FASE_SILENCIO_INICIAL){
        jogo->tempo_atual += delta_tempo;

        if(jogo->tempo_atual >= 0){
            jogo->fase = FASE_JOGANDO;
        }

        return;
    }

    if(!(jogo->fase == FASE_JOGANDO)){
        return;
    }

    Nota *nota = jogo->lista.cursor;

    while(nota != NULL){
        if(nota->estado == PENDENTE){
            if(jogo->tempo_atual > nota->tempo + JANELA_BOM){
                registrarErro(jogo, nota->injetada);
                atualizarEstadoNota(nota, ERRADO);
            }

            else{
                break;
            }
        }

        processarProximaNota(&jogo->lista);
        nota = jogo->lista.cursor;
    }
}