#include <stdio.h>
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