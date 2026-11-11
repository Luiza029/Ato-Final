#include <string.h>
#include "raylib.h"
#include "teclas.h"

int teclas_lanes[QUANTIDADE_LANES] = { KEY_D, KEY_F, KEY_J, KEY_K };
char rotulos_lanes[QUANTIDADE_LANES][4] = { "D", "F", "J", "K" };

/* Aceita letras, numeros e a tecla do Ç (KEY_SEMICOLON na raylib). */
static int teclaPermitida(int tecla)
{
    return (tecla >= KEY_A && tecla <= KEY_Z)
        || (tecla >= KEY_ZERO && tecla <= KEY_NINE)
        || tecla == KEY_SEMICOLON;
}

/* Devolve 1 se a tecla ja pertence a outra lane. */
static int teclaEmUso(int tecla, int lane_atual)
{
    for (int i = 0; i < QUANTIDADE_LANES; i++)
    {
        if (i != lane_atual && teclas_lanes[i] == tecla)
        {
            return 1;
        }
    }
    return 0;
}

/* Escreve em rotulo o texto da tecla que aparece na tela. */
static void montarRotulo(int tecla, char *rotulo)
{
    if (tecla == KEY_SEMICOLON)
    {
        strcpy(rotulo, "Ç");
    }
    else
    {
        rotulo[0] = (char)tecla;
        rotulo[1] = '\0';
    }
}

void restaurarTeclasPadrao(void)
{
    int padrao[QUANTIDADE_LANES] = { KEY_D, KEY_F, KEY_J, KEY_K };

    for (int i = 0; i < QUANTIDADE_LANES; i++)
    {
        teclas_lanes[i] = padrao[i];
        montarRotulo(padrao[i], rotulos_lanes[i]);
    }
}

/* Le a tecla apertada neste frame; so troca se for permitida e livre. */
ResultadoTecla tentarDefinirTeclaLane(int lane)
{
    int tecla = GetKeyPressed();

    if (tecla == 0)
    {
        return TECLA_NENHUMA;
    }
    if (!teclaPermitida(tecla) || teclaEmUso(tecla, lane))
    {
        return TECLA_REJEITADA;
    }

    teclas_lanes[lane] = tecla;
    montarRotulo(tecla, rotulos_lanes[lane]);
    return TECLA_ACEITA;
}