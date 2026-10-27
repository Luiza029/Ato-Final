#ifndef JOGO_H
#define JOGO_H

#include "listas_notas.h"

/* Fases pelas quais a partida passa */
typedef enum FasePartida{
    FASE_SILENCIO_INICIAL,
    FASE_JOGANDO,
    FASE_REWIND,
    FASE_VITORIA,
    FASE_DERROTA
} FasePartida;

/* Tudo o que muda durante uma partida */
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
} EstadoJogo;

/* Prepara a partida: zera o estado, carrega o mapa e ordena as notas.
   Retorna 1 se deu certo e 0 se nao conseguiu carregar o mapa. */
int iniciarJogo(EstadoJogo *jogo, const char *caminho_mapa);

/* Atualiza a partida a cada frame: avanca o tempo, marca notas perdidas
   e move o assassino. delta_tempo e o tempo do ultimo frame em segundos. */
void atualizarJogo(EstadoJogo *jogo, float delta_tempo);

/* Trata o aperto da tecla de uma lane: busca a nota e julga o acerto. */
void tratarTeclaLane(EstadoJogo *jogo, int lane);

/* Gasta um rewind e volta algumas notas, se ainda houver rewinds. */
void usarRewind(EstadoJogo *jogo);

/* Insere notas injetadas pelo assassino e reordena a lista. */
void injetarNotas(EstadoJogo *jogo);

/* Define vitoria ou derrota quando as notas acabam ou o assassino alcanca. */
void verificarFimDeJogo(EstadoJogo *jogo);

/* Libera a memoria da partida. */
void encerrarJogo(EstadoJogo *jogo);

#endif