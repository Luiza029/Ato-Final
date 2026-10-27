#ifndef INTERFACE_H
#define INTERFACE_H

#include "jogo.h"

void desenharJogo(const EstadoJogo *jogo);
void desenharCena(const EstadoJogo *jogo);
void desenharBarraInfo(const EstadoJogo *jogo);
void desenharPauta(const EstadoJogo *jogo);
void desenharNota(const Nota *nota, float tempo_atual);
void desenharFeedback(const EstadoJogo *jogo);
void desenharTelaFinal(const EstadoJogo *jogo);

#endif