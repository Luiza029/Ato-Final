#include <stddef.h>
#include "raylib.h"
#include "interface.h"
#include "constantes.h"
#include "constantes_raylib.h"

/* ---------- Funções auxiliares (privadas deste arquivo) ---------- */

/* Posição X de uma nota na tela, a partir do tempo dela e do tempo da música. */
static float calcularPosicaoX(float tempo_nota, float tempo_atual){
    return POSICAO_X_LINHA_ACERTO + (tempo_nota - tempo_atual) * VELOCIDADE_NOTAS;
}

/* Posição Y do centro de uma lane dentro da pauta. */
static int calcularCentroYLane(int lane){
    return POSICAO_Y_PAUTA + lane * ALTURA_LANE + ALTURA_LANE / 2;
}

/* Desenha um texto centralizado horizontalmente em centro_x. */
static void desenharTextoCentralizado(const char *texto, int centro_x, int posicao_y, int tamanho, Color cor){
    int largura_texto = MeasureText(texto, tamanho);
    DrawText(texto, centro_x - largura_texto / 2, posicao_y, tamanho, cor);
}

/* Efeito de fita VHS rebobinando: película escura + linhas que correm. */
static void desenharEfeitoRewind(void){
    DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_JANELA, Fade(BLACK, OPACIDADE_ESCURECER_REWIND));

    int deslocamento = (int)(GetTime() * VELOCIDADE_LINHAS_REWIND) % ESPACO_LINHAS_REWIND;
    for(int linha_y = deslocamento; linha_y < ALTURA_JANELA; linha_y += ESPACO_LINHAS_REWIND){
        DrawLine(0, linha_y, LARGURA_JANELA, linha_y, Fade(WHITE, OPACIDADE_LINHAS_REWIND));
    }

    desenharTextoCentralizado("<< REWIND", LARGURA_JANELA / 2, ALTURA_CENA / 2, TAMANHO_FONTE_TITULO, COR_TEXTO);
}

/* ---------- Funções públicas (declaradas em interface.h) ---------- */

/* Cena de cima: sobrevivente e assassino. Por enquanto, retângulos no lugar da pixelart. */
void desenharCena(const EstadoJogo *jogo){
    DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_CENA, COR_FUNDO_CENA);
    DrawRectangle(0, CHAO_Y, LARGURA_JANELA, ALTURA_CENA - CHAO_Y, COR_LINHA_LANE);

    DrawRectangle(SOBREVIVENTE_X, CHAO_Y - SOBREVIVENTE_ALTURA, SOBREVIVENTE_LARGURA, SOBREVIVENTE_ALTURA, SKYBLUE);

    /* perigo vai de 0 (assassino longe) a 1 (assassino colado no sobrevivente) */
    float perigo = (float)(DISTANCIA_MAXIMA_ASSASSINO - jogo->distancia_assassino)
                 / (float)(DISTANCIA_MAXIMA_ASSASSINO - DISTANCIA_MINIMA_ASSASSINO);
    int assassino_x = ASSASSINO_X_LONGE + (int)((ASSASSINO_X_PERTO - ASSASSINO_X_LONGE) * perigo);

    DrawRectangle(assassino_x, CHAO_Y - ASSASSINO_ALTURA, ASSASSINO_LARGURA, ASSASSINO_ALTURA, COR_TEXTO_DESTAQUE);
}

/* Barra do meio: pontos, combo, rewinds e distância do assassino. */
void desenharBarraInfo(const EstadoJogo *jogo){
    DrawRectangle(0, POSICAO_Y_BARRA_INFO, LARGURA_JANELA, ALTURA_BARRA_INFO, COR_FUNDO_BARRA_INFO);

    int texto_y = POSICAO_Y_BARRA_INFO + (ALTURA_BARRA_INFO - TAMANHO_FONTE_INFO) / 2;
    int largura_coluna = LARGURA_JANELA / 4;

    DrawText(TextFormat("Pontos: %d", jogo->pontuacao),
             MARGEM_TELA, texto_y, TAMANHO_FONTE_INFO, COR_TEXTO);
    DrawText(TextFormat("Combo: %d (x%d)", jogo->combo, jogo->multiplicador),
             largura_coluna + MARGEM_TELA, texto_y, TAMANHO_FONTE_INFO, COR_TEXTO);
    DrawText(TextFormat("Rewinds: %d", jogo->rewinds_restantes),
             2 * largura_coluna + MARGEM_TELA, texto_y, TAMANHO_FONTE_INFO, COR_TEXTO);

    /* Distância: um quadradinho por passo; os cheios mostram quanto falta para ser pego */
    const char *rotulo = "Distancia:";
    int pip_x = 3 * largura_coluna + MARGEM_TELA;
    DrawText(rotulo, pip_x, texto_y, TAMANHO_FONTE_INFO, COR_TEXTO);
    pip_x += MeasureText(rotulo, TAMANHO_FONTE_INFO) + MARGEM_TELA / 2;

    int pip_y = POSICAO_Y_BARRA_INFO + (ALTURA_BARRA_INFO - TAMANHO_PIP_DISTANCIA) / 2;
    for(int passo = 0; passo < DISTANCIA_MAXIMA_ASSASSINO; passo++){
        Color cor_pip = (passo < jogo->distancia_assassino) ? COR_BOM : COR_LINHA_LANE;
        DrawRectangle(pip_x + passo * (TAMANHO_PIP_DISTANCIA + ESPACO_PIP_DISTANCIA),
                      pip_y, TAMANHO_PIP_DISTANCIA, TAMANHO_PIP_DISTANCIA, cor_pip);
    }
}

/* Desenha uma nota. Notas já acertadas não aparecem. */
void desenharNota(const Nota *nota, float tempo_atual){
    if(nota->estado == PERFEITO || nota->estado == BOM){
        return;
    }

    int nota_x = (int)calcularPosicaoX(nota->tempo, tempo_atual);
    int nota_y = calcularCentroYLane(nota->lane);
    int raio = TAMANHO_NOTA / 2;

    Color cor_miolo = nota->injetada ? COR_NOTA_INJETADA : CORES_LANES[nota->lane];
    Color cor_borda = nota->injetada ? COR_BORDA_INJETADA : cor_miolo;

    if(nota->estado == ERRADO){
        cor_miolo = Fade(cor_miolo, OPACIDADE_NOTA_ERRADA);
        cor_borda = Fade(cor_borda, OPACIDADE_NOTA_ERRADA);
    }

    DrawCircle(nota_x, nota_y, raio, cor_borda);
    DrawCircle(nota_x, nota_y, raio - ESPESSURA_BORDA_NOTA, cor_miolo);
}

/* Pauta: fundo, lanes, linha de acerto e as notas (percorrendo a lista a partir do cursor). */
void desenharPauta(const EstadoJogo *jogo){
    DrawRectangle(0, POSICAO_Y_PAUTA, LARGURA_JANELA, ALTURA_PAUTA, COR_FUNDO_PAUTA);

    for(int lane = 0; lane < QUANTIDADE_LANES; lane++){
        int centro_y = calcularCentroYLane(lane);
        DrawLine(0, centro_y, LARGURA_JANELA, centro_y, COR_LINHA_LANE);
        DrawCircleLines(POSICAO_X_LINHA_ACERTO, centro_y, TAMANHO_NOTA / 2, COR_LINHA_ACERTO);
        DrawText(ROTULOS_LANES[lane], MARGEM_TELA, centro_y - TAMANHO_FONTE_INFO / 2,
                 TAMANHO_FONTE_INFO, COR_TEXTO);
    }

    DrawRectangle(POSICAO_X_LINHA_ACERTO - ESPESSURA_LINHA_ACERTO / 2, POSICAO_Y_PAUTA,
                  ESPESSURA_LINHA_ACERTO, ALTURA_PAUTA, COR_LINHA_ACERTO);

    /* A lista está ordenada por tempo: ao achar uma nota fora da tela pela direita,
       todas as seguintes também estão, então podemos parar. */
    const Nota *atual = jogo->lista.cursor;
    while(atual != NULL){
        if(calcularPosicaoX(atual->tempo, jogo->tempo_atual) - TAMANHO_NOTA > LARGURA_JANELA){
            break;
        }
        desenharNota(atual, jogo->tempo_atual);
        atual = atual->proximo;
    }
}

/* Texto PERFEITO / BOM / ERROU que some aos poucos. Supõe tempo_feedback em contagem regressiva. */
void desenharFeedback(const EstadoJogo *jogo){
    if(jogo->tempo_feedback <= 0.0f){
        return;
    }

    const char *texto;
    Color cor;

    switch(jogo->ultimo_julgamento){
        case PERFEITO: texto = "PERFEITO!"; cor = COR_PERFEITO; break;
        case BOM:      texto = "BOM";       cor = COR_BOM;      break;
        case ERRADO:   texto = "ERROU";     cor = COR_ERRADO;   break;
        default:       return;
    }

    float opacidade = jogo->tempo_feedback / DURACAO_FEEDBACK;
    desenharTextoCentralizado(texto, LARGURA_JANELA / 2, ALTURA_CENA / 4,
                              TAMANHO_FONTE_FEEDBACK, Fade(cor, opacidade));
}

/* Tela completa da partida. Quem chama é responsável por BeginDrawing/EndDrawing. */
void desenharJogo(const EstadoJogo *jogo){
    desenharCena(jogo);
    desenharBarraInfo(jogo);
    desenharPauta(jogo);
    desenharFeedback(jogo);

    if(jogo->fase == FASE_REWIND){
        desenharEfeitoRewind();
    }
}

/* Tela de vitória ou derrota, com pontuação e percentual de acertos. */
void desenharTelaFinal(const EstadoJogo *jogo){
    int centro_x = LARGURA_JANELA / 2;
    int venceu = (jogo->fase == FASE_VITORIA);
    int foi_pego = (jogo->distancia_assassino <= DISTANCIA_MINIMA_ASSASSINO);

    const char *titulo;
    if(venceu){
        titulo = "VOCE ESCAPOU";
    }else if(foi_pego){
        titulo = "O ASSASSINO TE PEGOU";
    }else{
        titulo = "A FUGA FALHOU";
    }

    DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_JANELA, COR_FUNDO_CENA);
    desenharTextoCentralizado(titulo, centro_x, ALTURA_JANELA / 5, TAMANHO_FONTE_TITULO,
                              venceu ? COR_PERFEITO : COR_TEXTO_DESTAQUE);

    float percentual = calcularPercentualAcertos(&jogo->lista);

    desenharTextoCentralizado(TextFormat("Pontuacao: %d", jogo->pontuacao),
                              centro_x, ALTURA_JANELA / 2 - TAMANHO_FONTE_TEXTO,
                              TAMANHO_FONTE_TEXTO, COR_TEXTO);
    desenharTextoCentralizado(TextFormat("Acertos: %.0f%% (minimo: %.0f%%)",
                              percentual * 100.0f, PERCENTUAL_MINIMO_VITORIA * 100.0f),
                              centro_x, ALTURA_JANELA / 2 + TAMANHO_FONTE_TEXTO,
                              TAMANHO_FONTE_TEXTO, COR_TEXTO);
    desenharTextoCentralizado("ENTER para voltar ao menu", centro_x, ALTURA_JANELA * 4 / 5,
                              TAMANHO_FONTE_TEXTO, COR_TEXTO);
}