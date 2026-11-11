#ifndef TECLAS_H
#define TECLAS_H

#include "constantes.h"

typedef enum ResultadoTecla{
    TECLA_NENHUMA,
    TECLA_ACEITA,
    TECLA_REJEITADA
} ResultadoTecla;

extern int teclas_lanes[QUANTIDADE_LANES];
extern char rotulos_lanes[QUANTIDADE_LANES][4];

/* Volta as teclas para D F J K. */
void restaurarTeclasPadrao(void);

/* Le a tecla apertada neste frame e tenta atribuir a lane. */
ResultadoTecla tentarDefinirTeclaLane(int lane);

/* Le as teclas salvas em CAMINHO_TECLAS; se faltar ou for invalido, usa D F J K. */
void carregarTeclas(void);

/* Grava as teclas atuais em CAMINHO_TECLAS. */
void salvarTeclas(void);

#endif