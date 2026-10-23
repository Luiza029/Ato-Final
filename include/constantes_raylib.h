#ifndef CONSTANTES_RAYLIB_H
#define CONSTANTES_RAYLIB_H

/*
 * Constantes que dependem da raylib (teclas e cores).
 * Só devem ser incluídas por jogo.c, interface.c, menu.c e main.c.
 * NUNCA incluir em lista_notas.c, ordenacao.c ou mapa.c.
 */

#include "raylib.h"
#include "constantes.h"   // precisa de QUANTIDADE_LANES

/* ---------- TECLAS ---------- */

// Uma tecla por lane, na ordem das lanes (0 a 3).
// Uso: IsKeyPressed(TECLAS_LANES[lane])
static const int TECLAS_LANES[QUANTIDADE_LANES] = { KEY_D, KEY_F, KEY_J, KEY_K };

// Rótulo desenhado embaixo de cada lane, na mesma ordem das teclas acima
static const char *const ROTULOS_LANES[QUANTIDADE_LANES] = { "D", "F", "J", "K" };

#define TECLA_REWIND      KEY_SPACE
#define TECLA_CONFIRMAR   KEY_ENTER
#define TECLA_VOLTAR      KEY_BACKSPACE
#define TECLA_MENU_CIMA   KEY_UP
#define TECLA_MENU_BAIXO  KEY_DOWN

/* ---------- CORES ---------- */
// Formato: (Color){ vermelho, verde, azul, transparência }, cada valor de 0 a 255

#define COR_FUNDO_CENA        (Color){ 12, 10, 18, 255 }
#define COR_FUNDO_PAUTA       (Color){ 20, 18, 28, 255 }
#define COR_FUNDO_BARRA_INFO  (Color){ 35, 30, 45, 255 }
#define COR_LINHA_LANE        (Color){ 70, 65, 85, 255 }
#define COR_LINHA_ACERTO      (Color){ 230, 230, 230, 255 }
#define COR_TEXTO             (Color){ 235, 235, 235, 255 }
#define COR_TEXTO_DESTAQUE    (Color){ 200, 30, 30, 255 }   // vermelho sangue

// Notas normais: uma cor por lane
static const Color CORES_LANES[QUANTIDADE_LANES] = {
    { 220, 80, 80, 255 },
    { 230, 180, 60, 255 },
    { 80, 180, 220, 255 },
    { 150, 100, 220, 255 }
};

// Notas injetadas pelo assassino (campo "injetada" da struct Nota)
#define COR_NOTA_INJETADA     (Color){ 255, 255, 255, 255 }
#define COR_BORDA_INJETADA    (Color){ 200, 30, 30, 255 }

// Feedback de julgamento
#define COR_PERFEITO          (Color){ 255, 215, 0, 255 }
#define COR_BOM               (Color){ 90, 220, 120, 255 }
#define COR_ERRADO            (Color){ 220, 50, 50, 255 }

#endif