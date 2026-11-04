#include "jogo.h"
#include "constantes.h"
#include "listas_notas.h"
#include "mapa.h"
#include "ordenacao.h"
#include <math.h>
#include <stdio.h>

int iniciarJogo(EstadoJogo *jogo, const char *caminho_mapa) {
  criarLista(&jogo->lista);

  if (carregarMapa(&jogo->lista, caminho_mapa) == 0) {
    return 0;
  }

  jogo->pontuacao = 0;
  jogo->combo = 0;
  jogo->erros_acumulados = 0;
  jogo->acertos_seguidos = 0;

  jogo->multiplicador = 1;
  jogo->distancia_assassino = DISTANCIA_INICIAL_ASSASSINO;
  jogo->rewinds_restantes = REWINDS_MAXIMOS;
  jogo->tempo_atual = -SILENCIO_INICIAL;

  jogo->fase = FASE_SILENCIO_INICIAL;
  jogo->temporizador_fase = 0;

  jogo->ultimo_julgamento = PENDENTE;
  jogo->tempo_feedback = 0;
  jogo->tempo_inicio_rewind = 0;
  jogo->tempo_alvo_rewind = 0;

  return 1;
}

void encerrarJogo(EstadoJogo *jogo) { liberarLista(&jogo->lista); }

static void registrarAcerto(EstadoJogo *jogo, EstadoNota resultado) {
  jogo->combo++;
  jogo->multiplicador = 1 + jogo->combo / COMBO_POR_MULTIPLICADOR;

  if (jogo->multiplicador > MULTIPLICADOR_MAXIMO) {
    jogo->multiplicador = MULTIPLICADOR_MAXIMO;
  }

  if (resultado == PERFEITO) {
    jogo->pontuacao += PONTOS_PERFEITO * jogo->multiplicador;
  }

  else {
    jogo->pontuacao += PONTOS_BOM * jogo->multiplicador;
  }

  jogo->acertos_seguidos++;

  if (jogo->acertos_seguidos == ACERTOS_PARA_RECUPERAR) {
    jogo->distancia_assassino += DISTANCIA_GANHA_POR_RECUPERACAO;

    if (jogo->distancia_assassino > DISTANCIA_MAXIMA_ASSASSINO) {
      jogo->distancia_assassino = DISTANCIA_MAXIMA_ASSASSINO;
    }

    jogo->acertos_seguidos = 0;
  }

  jogo->ultimo_julgamento = resultado;
  jogo->tempo_feedback = DURACAO_FEEDBACK;
}

static void registrarErro(EstadoJogo *jogo, int nota_era_injetada) {
  jogo->combo = 0;
  jogo->acertos_seguidos = 0;
  jogo->multiplicador = 1;

  jogo->distancia_assassino -= DISTANCIA_PERDIDA_POR_ERRO;

  if (jogo->distancia_assassino < DISTANCIA_MINIMA_ASSASSINO) {
    jogo->distancia_assassino = DISTANCIA_MINIMA_ASSASSINO;
  }

  if (!nota_era_injetada) {
    jogo->erros_acumulados++;
  }

  jogo->ultimo_julgamento = ERRADO;
  jogo->tempo_feedback = DURACAO_FEEDBACK;
}

void tratarTeclaLane(EstadoJogo *jogo, int lane) {
  if (!(jogo->fase == FASE_JOGANDO)) {
    return;
  }

  Nota *nota = buscarNota(&jogo->lista, lane, jogo->tempo_atual);

  if (nota == NULL) {
    registrarErro(jogo, 0);
    return;
  }

  float diferenca = fabsf(nota->tempo - jogo->tempo_atual);
  EstadoNota resultado;

  if (diferenca <= JANELA_PERFEITO) {
    resultado = PERFEITO;
  }

  else {
    resultado = BOM;
  }

  atualizarEstadoNota(nota, resultado);

  registrarAcerto(jogo, resultado);
}

void verificarFimDeJogo(EstadoJogo *jogo) {
  if (jogo->fase != FASE_JOGANDO) {
    return;
  }

  if (jogo->distancia_assassino <= DISTANCIA_MINIMA_ASSASSINO) {
    jogo->fase = FASE_DERROTA;
    return;
  }

  if (jogo->lista.cursor != NULL) {
    return;
  }

  float tempo_ultima_nota = 0.0f;
  if (jogo->lista.fim != NULL) {
    tempo_ultima_nota = jogo->lista.fim->tempo;
  }

  if (jogo->tempo_atual < tempo_ultima_nota + ESPERA_FIM_DA_MUSICA) {
    return;
  }

  if (calcularPercentualAcertos(&jogo->lista) >= PERCENTUAL_MINIMO_VITORIA) {
    jogo->fase = FASE_VITORIA;
  }

  else {
    jogo->fase = FASE_DERROTA;
  }
}

void injetarNotas(EstadoJogo *jogo) {
  if (jogo->erros_acumulados < ERROS_PARA_INJECAO) {
    return;
  }

  for (int i = 0; i < NOTAS_POR_INJECAO; i++) {
    float tempo = jogo->tempo_atual + ANTECEDENCIA_INJECAO +
                  INTERVALO_NOTAS_INJETADAS * i;
    int lane = i % QUANTIDADE_LANES;
    inserirNota(&jogo->lista, tempo, lane, 1);
  }

  ordenarNotas(&jogo->lista);
  jogo->erros_acumulados = 0;
}

static void descontarPontosDoRewind(EstadoJogo *jogo) {
  Nota *nota = jogo->lista.cursor;

  for (int i = 0; i < NOTAS_POR_REWIND; i++) {
    if (nota == NULL) {
      nota = jogo->lista.fim;
    }

    else if (nota->anterior != NULL) {
      nota = nota->anterior;
    }

    else {
      break;
    }

    if (nota == NULL) {
      break;
    }

    if (nota->estado == PERFEITO) {
      jogo->pontuacao -= PONTOS_PERFEITO;
    }

    else if (nota->estado == BOM) {
      jogo->pontuacao -= PONTOS_BOM;
    }
  }
}

void usarRewind(EstadoJogo *jogo) {
  if (jogo->fase != FASE_JOGANDO || jogo->rewinds_restantes <= 0) {
    return;
  }

  Nota *cursor_antes = jogo->lista.cursor;

  descontarPontosDoRewind(jogo);
  voltarCursor(&jogo->lista, NOTAS_POR_REWIND);

  // Nada para voltar (lista vazia ou cursor ja no inicio): nao gasta o rewind.
  if (jogo->lista.cursor == cursor_antes) {
    return;
  }

  jogo->rewinds_restantes--;

  jogo->distancia_assassino = DISTANCIA_MAXIMA_ASSASSINO;
  jogo->combo = 0;
  jogo->multiplicador = 1;
  jogo->acertos_seguidos = 0;

  jogo->tempo_inicio_rewind = jogo->tempo_atual;
  jogo->tempo_alvo_rewind = jogo->lista.cursor->tempo - RESPIRO_APOS_REWIND;

  jogo->fase = FASE_REWIND;
  jogo->temporizador_fase = 0;
}

void atualizarJogo(EstadoJogo *jogo, float delta_tempo) {
  if (jogo->tempo_feedback > 0) {
    jogo->tempo_feedback -= delta_tempo;
  }

  if (jogo->fase == FASE_SILENCIO_INICIAL) {
    jogo->tempo_atual += delta_tempo;

    if (jogo->tempo_atual >= 0) {
      jogo->fase = FASE_JOGANDO;
    }

    return;
  }

  if (jogo->fase == FASE_REWIND) {
    jogo->temporizador_fase += delta_tempo;

    float progresso = jogo->temporizador_fase / DURACAO_ANIMACAO_REWIND;

    if (progresso >= 1.0f) {
      jogo->tempo_atual = jogo->tempo_alvo_rewind;
      jogo->fase = FASE_JOGANDO;
    }

    else {
      jogo->tempo_atual = jogo->tempo_inicio_rewind +
      (jogo->tempo_alvo_rewind - jogo->tempo_inicio_rewind) * progresso;
    }

    return;
  }

  Nota *nota = jogo->lista.cursor;

  while (nota != NULL) {
    if (nota->estado == PENDENTE) {
      if (jogo->tempo_atual > nota->tempo + JANELA_BOM) {
        registrarErro(jogo, nota->injetada);
        atualizarEstadoNota(nota, ERRADO);
      }

      else {
        break;
      }
    }

    processarProximaNota(&jogo->lista);
    nota = jogo->lista.cursor;
  }

  injetarNotas(jogo);
  verificarFimDeJogo(jogo);
}
