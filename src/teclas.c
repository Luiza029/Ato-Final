#include <string.h>
#include "raylib.h"
#include "teclas.h"
#include <stdio.h>

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

/* Le as 4 teclas do arquivo; se algo estiver errado, volta para as teclas padrao. */
void carregarTeclas(void)
{
    FILE *arq = fopen(CAMINHO_TECLAS, "r");
    int lidas[QUANTIDADE_LANES];
    int validas = (arq != NULL);

    for (int i = 0; i < QUANTIDADE_LANES && validas; i++)
    {
        validas = (fscanf(arq, "%d", &lidas[i]) == 1);
    }

    if (arq != NULL)
    {
        fclose(arq);
    }

    for (int i = 0; i < QUANTIDADE_LANES && validas; i++)
    {
        if (!teclaPermitida(lidas[i]))
        {
            validas = 0;
        }
        for (int j = 0; j < i; j++)
        {
            if (lidas[j] == lidas[i])
            {
                validas = 0;
            }
        }
    }

    if (!validas)
    {
        restaurarTeclasPadrao();
        return;
    }

    for (int i = 0; i < QUANTIDADE_LANES; i++)
    {
        teclas_lanes[i] = lidas[i];
        montarRotulo(lidas[i], rotulos_lanes[i]);
    }
}

/* Grava os codigos das 4 teclas em uma linha, separados por espaco. */
void salvarTeclas(void)
{
    FILE *arq = fopen(CAMINHO_TECLAS, "w");

    if (arq != NULL)
    {
        for (int i = 0; i < QUANTIDADE_LANES; i++)
        {
            fprintf(arq, "%d%c", teclas_lanes[i], i == QUANTIDADE_LANES - 1 ? '\n' : ' ');
        }
        fclose(arq);
    }
}