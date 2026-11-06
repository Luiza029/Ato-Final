#ifndef JOGO_H
#define JOGO_H
#include "listas_notas.h"
#include "constantes.h"

typedef enum FasePartida{
    FASE_SILENCIO_INICIAL,
    FASE_JOGANDO,
    FASE_REWIND,
    FASE_VITORIA,
    FASE_DERROTA
} FasePartida;

typedef struct EstadoJogo{
    Listas_notas lista;          
    FasePartida fase;
    float tempo_atual;           
    float temporizador_fase;     
    int pontuacao;
    int combo;
    int multiplicador;
    int distancia_assassino;     
    int erros_acumulados;        
    int acertos_seguidos;       
    int rewinds_restantes;
    EstadoNota ultimo_julgamento;
    float tempo_feedback;
    float tempo_inicio_rewind;
    float tempo_alvo_rewind;

    /* Respingo de sangue na linha de acerto, um por lane */
    float tempo_efeito_lane[QUANTIDADE_LANES];     // contagem regressiva do respingo (0 = sem efeito)
    EstadoNota resultado_efeito_lane[QUANTIDADE_LANES];  // PERFEITO faz um respingo maior que BOM
    unsigned int semente_efeito_lane[QUANTIDADE_LANES];  // muda o desenho de cada respingo    

    /* Notas longas: a que esta sendo segurada em cada lane (NULL = nenhuma) e como foi o acerto
       da cabeca dela. A nota so vira PERFEITO ou BOM quando o jogador segura ate o fim. */
    Nota *nota_segurada[QUANTIDADE_LANES];
    EstadoNota resultado_cabeca[QUANTIDADE_LANES];
} EstadoJogo;

int iniciarJogo(EstadoJogo *jogo, const char *caminho_mapa);
void atualizarJogo(EstadoJogo *jogo, float delta_tempo);
void tratarTeclaLane(EstadoJogo *jogo, int lane);
/* Avisa que a tecla da lane foi solta: se havia nota longa segurada antes do fim, e erro. */
void soltarTeclaLane(EstadoJogo *jogo, int lane);
void usarRewind(EstadoJogo *jogo);
void injetarNotas(EstadoJogo *jogo);
void verificarFimDeJogo(EstadoJogo *jogo);
void encerrarJogo(EstadoJogo *jogo);

#endif