#include <stddef.h>
#include <math.h>
#include "raylib.h"
#include "interface.h"
#include "constantes.h"
#include "constantes_raylib.h"
#include "teclas.h"

static Texture2D textura_icone_rewind;
static bool tem_icone_rewind = false;

/* Sprite de um personagem: poses de corrida lado a lado na mesma imagem. recorte e a
   parte de cada quadro que tem desenho (sem a sobra transparente), igual em todos os quadros
   para o personagem nao "tremer" de uma pose para a outra. */
typedef struct SpritePersonagem{
    Texture2D textura;
    Rectangle recorte;
    int quadros;
    bool carregado;
} SpritePersonagem;

static SpritePersonagem sprite_assassino;
static SpritePersonagem sprite_sobrevivente;

/* Fundo da cena em camadas, da mais distante para a mais proxima. Cada camada anda numa
   velocidade (as distantes mais devagar), o que da a sensacao de que os dois estao correndo. */
typedef enum CamadaCena{
    CAMADA_CEU,
    CAMADA_PREDIOS_LONGE,
    CAMADA_PREDIOS_PERTO,
    CAMADA_NEBLINA,
    CAMADA_CHAO,
    QUANTIDADE_CAMADAS_CENA
} CamadaCena;

static const char *CAMINHOS_CAMADAS_CENA[QUANTIDADE_CAMADAS_CENA] = {
    CAMINHO_CENA_CEU,
    CAMINHO_CENA_PREDIOS_LONGE,
    CAMINHO_CENA_PREDIOS_PERTO,
    CAMINHO_CENA_NEBLINA,
    CAMINHO_CENA_CHAO
};

static const float VELOCIDADES_CAMADAS_CENA[QUANTIDADE_CAMADAS_CENA] = {
    VELOCIDADE_CENA_CEU,
    VELOCIDADE_CENA_PREDIOS_LONGE,
    VELOCIDADE_CENA_PREDIOS_PERTO,
    VELOCIDADE_CENA_NEBLINA,
    VELOCIDADE_CENA_CHAO
};

static Texture2D texturas_camadas_cena[QUANTIDADE_CAMADAS_CENA];
static bool tem_camada_cena[QUANTIDADE_CAMADAS_CENA] = { false };

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

/* Número entre 0 e 1 a partir de uma semente (gerador congruente linear simples).
   Cada respingo guarda a sua semente, então as gotas ficam iguais em todos os quadros
   do mesmo respingo, mas cada acerto tem um desenho diferente. */
static float sortearDaSemente(unsigned int *semente){
    *semente = *semente * 1103515245u + 12345u;
    return (float)((*semente >> 16) & 0x7FFF) / 32767.0f;
}

/* Respingo de sangue: uma mancha que cresce e some, e gotas que voam e caem. */
static void desenharRespingoSangue(int lane, float tempo_restante, EstadoNota resultado, unsigned int semente){
    float tempo_passado = DURACAO_EFEITO_SANGUE - tempo_restante;
    float progresso = tempo_passado / DURACAO_EFEITO_SANGUE;   // 0 no acerto, 1 no fim
    float opacidade = 1.0f - progresso;

    Vector2 centro = { (float)POSICAO_X_LINHA_ACERTO, (float)calcularCentroYLane(lane) };

    float raio_mancha = RAIO_MANCHA_SANGUE * (0.6f + 0.4f * progresso);
    DrawCircleV(centro, raio_mancha, Fade(COR_SANGUE_ESCURO, OPACIDADE_MANCHA_SANGUE * opacidade));

    int quantidade_gotas = (resultado == PERFEITO) ? GOTAS_SANGUE_PERFEITO : GOTAS_SANGUE_BOM;

    for(int gota = 0; gota < quantidade_gotas; gota++){
        float angulo = sortearDaSemente(&semente) * 2.0f * PI;
        float velocidade = VELOCIDADE_MINIMA_GOTA + sortearDaSemente(&semente) * (VELOCIDADE_MAXIMA_GOTA - VELOCIDADE_MINIMA_GOTA);
        float raio = RAIO_MINIMO_GOTA + sortearDaSemente(&semente) * (RAIO_MAXIMO_GOTA - RAIO_MINIMO_GOTA);

        Vector2 posicao = {
            centro.x + cosf(angulo) * velocidade * tempo_passado,
            centro.y + sinf(angulo) * velocidade * tempo_passado + 0.5f * GRAVIDADE_GOTA * tempo_passado * tempo_passado
        };

        DrawCircleV(posicao, raio * (1.0f - 0.5f * progresso), Fade(COR_SANGUE, opacidade));
    }
}

/* Quanto do mapa ja passou, de 0 a 1. A lista esta ordenada, entao o fim e a ultima nota. */
static float calcularProgressoMapa(const EstadoJogo *jogo){
    if(jogo->lista.fim == NULL || jogo->lista.fim->tempo <= 0.0f){
        return 0.0f;
    }

    float progresso = jogo->tempo_atual / jogo->lista.fim->tempo;

    if(progresso < 0.0f){
        return 0.0f;
    }
    if(progresso > 1.0f){
        return 1.0f;
    }
    return progresso;
}

/* Pontuacao no canto da cena, com o multiplicador do combo ao lado. */
static void desenharPontuacao(const EstadoJogo *jogo){
    const char *texto_pontos = TextFormat("Pontos: %d", jogo->pontuacao);
    DrawText(texto_pontos, MARGEM_TELA, MARGEM_TELA, TAMANHO_FONTE_PONTOS, COR_TEXTO);

    int multiplicador_x = MARGEM_TELA + MeasureText(texto_pontos, TAMANHO_FONTE_PONTOS) + MARGEM_TELA / 2;
    Color cor_multiplicador = (jogo->multiplicador > 1) ? COR_PERFEITO : Fade(COR_TEXTO, OPACIDADE_OPCAO_MENU);
    DrawText(TextFormat("x%d", jogo->multiplicador), multiplicador_x, MARGEM_TELA, TAMANHO_FONTE_PONTOS, cor_multiplicador);
}

/* Barra de progresso do mapa: contorno do tamanho total e preenchimento do que ja passou. */
static void desenharProgressoMapa(const EstadoJogo *jogo){
    int barra_x = 2 * MARGEM_TELA;
    int barra_y = POSICAO_Y_BARRA_INFO + (ALTURA_BARRA_INFO - ALTURA_BARRA_PROGRESSO) / 2;
    int largura_preenchida = (int)(LARGURA_BARRA_PROGRESSO * calcularProgressoMapa(jogo));

    DrawRectangle(barra_x, barra_y, LARGURA_BARRA_PROGRESSO, ALTURA_BARRA_PROGRESSO, COR_FUNDO_PAUTA);
    DrawRectangle(barra_x, barra_y, largura_preenchida, ALTURA_BARRA_PROGRESSO, COR_TEXTO_DESTAQUE);
    DrawRectangleLines(barra_x, barra_y, LARGURA_BARRA_PROGRESSO, ALTURA_BARRA_PROGRESSO, COR_LINHA_LANE);
}

/* Uma fita VHS por rewind, alinhadas a direita. As fitas ja usadas ficam apagadas. */
static void desenharRewindsRestantes(const EstadoJogo *jogo){
    int texto_y = POSICAO_Y_BARRA_INFO + (ALTURA_BARRA_INFO - TAMANHO_FONTE_INFO) / 2;

    // Sem a imagem (arquivo faltando), mostra so o numero
    if(!tem_icone_rewind){
        const char *texto = TextFormat("Rewinds: %d", jogo->rewinds_restantes);
        DrawText(texto, LARGURA_JANELA - 2 * MARGEM_TELA - MeasureText(texto, TAMANHO_FONTE_INFO), texto_y, TAMANHO_FONTE_INFO, COR_TEXTO);
        return;
    }

    int largura_icone = textura_icone_rewind.width * ESCALA_ICONE_REWIND;
    int altura_icone = textura_icone_rewind.height * ESCALA_ICONE_REWIND;
    int largura_total = REWINDS_MAXIMOS * largura_icone + (REWINDS_MAXIMOS - 1) * ESPACO_ICONES_REWIND;
    int icones_x = LARGURA_JANELA - 2 * MARGEM_TELA - largura_total;
    int icone_y = POSICAO_Y_BARRA_INFO + (ALTURA_BARRA_INFO - altura_icone) / 2;

    const char *rotulo = "Rewind (ESPACO)";
    DrawText(rotulo, icones_x - MARGEM_TELA - MeasureText(rotulo, TAMANHO_FONTE_INFO), texto_y, TAMANHO_FONTE_INFO, Fade(COR_TEXTO, OPACIDADE_OPCAO_MENU));

    for(int rewind = 0; rewind < REWINDS_MAXIMOS; rewind++){
        Vector2 posicao = { (float)(icones_x + rewind * (largura_icone + ESPACO_ICONES_REWIND)), (float)icone_y };
        Color tinta = (rewind < jogo->rewinds_restantes) ? WHITE : Fade(WHITE, OPACIDADE_REWIND_GASTO);
        DrawTextureEx(textura_icone_rewind, posicao, 0.0f, (float)ESCALA_ICONE_REWIND, tinta);
    }
}

/* Carrega o sprite de um personagem e descobre o recorte: junta as areas com desenho de todos
   os quadros (GetImageAlphaBorder acha o retangulo sem a sobra transparente de cada um).
   Se o arquivo faltar, sprite->carregado fica false e o personagem vira um retangulo. */
static void carregarSpritePersonagem(SpritePersonagem *sprite, const char *caminho, int quadros){
    sprite->carregado = false;
    sprite->quadros = quadros;

    if(!FileExists(caminho)){
        return;
    }

    Image imagem = LoadImage(caminho);
    float largura_quadro = (float)(imagem.width / quadros);
    float esquerda = largura_quadro, topo = (float)imagem.height, direita = 0.0f, base = 0.0f;

    for(int quadro = 0; quadro < quadros; quadro++){
        Image pose = ImageFromImage(imagem, (Rectangle){ quadro * largura_quadro, 0.0f, largura_quadro, (float)imagem.height });
        Rectangle desenho = GetImageAlphaBorder(pose, TRANSPARENCIA_MINIMA_SPRITE);
        UnloadImage(pose);

        if(desenho.width <= 0.0f || desenho.height <= 0.0f){
            continue;
        }

        esquerda = fminf(esquerda, desenho.x);
        topo = fminf(topo, desenho.y);
        direita = fmaxf(direita, desenho.x + desenho.width);
        base = fmaxf(base, desenho.y + desenho.height);
    }

    if(direita > esquerda && base > topo){
        sprite->recorte = (Rectangle){ esquerda, topo, direita - esquerda, base - topo };
        sprite->textura = LoadTextureFromImage(imagem);
        // O desenho original e grande e vai ser reduzido: filtro bilinear para nao serrilhar
        SetTextureFilter(sprite->textura, TEXTURE_FILTER_BILINEAR);
        sprite->carregado = true;
    }

    UnloadImage(imagem);
}

static void descarregarSpritePersonagem(SpritePersonagem *sprite){
    if(sprite->carregado){
        UnloadTexture(sprite->textura);
        sprite->carregado = false;
    }
}

/* Largura do personagem na tela: o recorte do sprite reduzido para a altura pedida,
   mantendo a proporcao. Sem sprite, usa a largura do retangulo reserva. */
static float calcularLarguraPersonagem(const SpritePersonagem *sprite, float altura, float largura_sem_sprite){
    if(!sprite->carregado){
        return largura_sem_sprite;
    }

    return sprite->recorte.width * altura / sprite->recorte.height;
}

/* Desenha o personagem correndo com o pe no chao, a partir de x.
   No rewind a animacao roda de tras para frente, como uma fita voltando.
   Sem sprite, desenha um retangulo da cor reserva no lugar. */
static void desenharPersonagem(const SpritePersonagem *sprite, float x, float altura, float quadros_por_segundo,
                               int rebobinando, float largura_sem_sprite, Color cor_sem_sprite){
    if(!sprite->carregado){
        DrawRectangle((int)x, CHAO_Y - (int)altura, (int)largura_sem_sprite, (int)altura, cor_sem_sprite);
        return;
    }

    int quadro = (int)(GetTime() * quadros_por_segundo) % sprite->quadros;
    if(rebobinando){
        quadro = sprite->quadros - 1 - quadro;
    }

    float largura_quadro = (float)(sprite->textura.width / sprite->quadros);
    Rectangle origem = { quadro * largura_quadro + sprite->recorte.x, sprite->recorte.y,
                         sprite->recorte.width, sprite->recorte.height };
    Rectangle destino = { x, CHAO_Y - altura, calcularLarguraPersonagem(sprite, altura, largura_sem_sprite), altura };

    DrawTexturePro(sprite->textura, origem, destino, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
}

/* Carrega as camadas do fundo que existirem. Sem uma camada, o fundo simples continua no lugar. */
static void carregarFundoCena(void){
    for(int camada = 0; camada < QUANTIDADE_CAMADAS_CENA; camada++){
        if(!FileExists(CAMINHOS_CAMADAS_CENA[camada])){
            continue;
        }

        texturas_camadas_cena[camada] = LoadTexture(CAMINHOS_CAMADAS_CENA[camada]);
        // Pixel art ampliada sem borrar, e a imagem se repete quando passa da borda
        SetTextureFilter(texturas_camadas_cena[camada], TEXTURE_FILTER_POINT);
        SetTextureWrap(texturas_camadas_cena[camada], TEXTURE_WRAP_REPEAT);
        tem_camada_cena[camada] = true;
    }
}

static void descarregarFundoCena(void){
    for(int camada = 0; camada < QUANTIDADE_CAMADAS_CENA; camada++){
        if(tem_camada_cena[camada]){
            UnloadTexture(texturas_camadas_cena[camada]);
            tem_camada_cena[camada] = false;
        }
    }
}

/* Desenha uma camada esticada em destino, deslocada para a esquerda conforme o tempo.
   Como a textura repete (TEXTURE_WRAP_REPEAT), o que sai pela esquerda volta pela direita sem emenda.
   O tempo e o da partida: no rewind o cenario anda para tras, junto com a musica. */
static void desenharCamadaCena(CamadaCena camada, float tempo, Rectangle destino){
    Texture2D textura = texturas_camadas_cena[camada];
    Rectangle origem = { tempo * VELOCIDADES_CAMADAS_CENA[camada], 0.0f, (float)textura.width, (float)textura.height };

    DrawTexturePro(textura, origem, destino, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);
}

/* Fundo da cena: ceu, predios, neblina e calcada. Camada que nao existe vira o fundo simples. */
static void desenharFundoCena(float tempo){
    Rectangle cena_inteira = { 0.0f, 0.0f, (float)LARGURA_JANELA, (float)ALTURA_CENA };
    Rectangle faixa_chao = { 0.0f, (float)CHAO_Y, (float)LARGURA_JANELA, (float)(ALTURA_CENA - CHAO_Y) };

    if(!tem_camada_cena[CAMADA_CEU]){
        DrawRectangle(0, 0, LARGURA_JANELA, ALTURA_CENA, COR_FUNDO_CENA);
    }

    for(int camada = CAMADA_CEU; camada <= CAMADA_NEBLINA; camada++){
        if(tem_camada_cena[camada]){
            desenharCamadaCena((CamadaCena)camada, tempo, cena_inteira);
        }
    }

    if(tem_camada_cena[CAMADA_CHAO]){
        desenharCamadaCena(CAMADA_CHAO, tempo, faixa_chao);
    }
    else{
        DrawRectangle(0, CHAO_Y, LARGURA_JANELA, ALTURA_CENA - CHAO_Y, COR_LINHA_LANE);
    }
}

/* ---------- Funções públicas (declaradas em interface.h) ---------- */

void carregarInterface(void){
    if(FileExists(CAMINHO_ICONE_REWIND)){
        textura_icone_rewind = LoadTexture(CAMINHO_ICONE_REWIND);
        // Filtro "ponto": ao ampliar, cada pixel vira um quadrado nitido (pixel art nao fica borrada)
        SetTextureFilter(textura_icone_rewind, TEXTURE_FILTER_POINT);
        tem_icone_rewind = true;
    }

    carregarSpritePersonagem(&sprite_assassino, CAMINHO_SPRITE_ASSASSINO, QUADROS_SPRITE_ASSASSINO);
    carregarSpritePersonagem(&sprite_sobrevivente, CAMINHO_SPRITE_SOBREVIVENTE, QUADROS_SPRITE_SOBREVIVENTE);
    carregarFundoCena();
}

void descarregarInterface(void){
    if(tem_icone_rewind){
        UnloadTexture(textura_icone_rewind);
        tem_icone_rewind = false;
    }

    descarregarSpritePersonagem(&sprite_assassino);
    descarregarSpritePersonagem(&sprite_sobrevivente);

    descarregarFundoCena();
}

/* Cena de cima: sobrevivente, assassino e a pontuacao no canto. */
void desenharCena(const EstadoJogo *jogo){
    int rebobinando = jogo->fase == FASE_REWIND;

    desenharFundoCena(jogo->tempo_atual);

    desenharPersonagem(&sprite_sobrevivente, (float)SOBREVIVENTE_X, (float)SOBREVIVENTE_ALTURA, QUADROS_POR_SEGUNDO_SOBREVIVENTE,
                       rebobinando, (float)SOBREVIVENTE_LARGURA, SKYBLUE);

    /* perigo vai de 0 (assassino longe) a 1 (assassino colado no sobrevivente).
       Usa a distancia exibida, que anda aos poucos, para o assassino correr ate o lugar novo. */
    float perigo = (DISTANCIA_MAXIMA_ASSASSINO - jogo->distancia_exibida)
                 / (float)(DISTANCIA_MAXIMA_ASSASSINO - DISTANCIA_MINIMA_ASSASSINO);
    float largura_assassino = calcularLarguraPersonagem(&sprite_assassino, (float)ASSASSINO_ALTURA, (float)ASSASSINO_LARGURA);
    float assassino_x_perto = SOBREVIVENTE_X - largura_assassino;
    float assassino_x = ASSASSINO_X_LONGE + (assassino_x_perto - ASSASSINO_X_LONGE) * perigo;

    desenharPersonagem(&sprite_assassino, assassino_x, (float)ASSASSINO_ALTURA, QUADROS_POR_SEGUNDO_ASSASSINO,
                       rebobinando, (float)ASSASSINO_LARGURA, COR_TEXTO_DESTAQUE);

    desenharPontuacao(jogo);
}

/* Barra do meio: progresso do mapa a esquerda e as fitas de rewind a direita. */
void desenharBarraInfo(const EstadoJogo *jogo){
    DrawRectangle(0, POSICAO_Y_BARRA_INFO, LARGURA_JANELA, ALTURA_BARRA_INFO, COR_FUNDO_BARRA_INFO);

    desenharProgressoMapa(jogo);
    desenharRewindsRestantes(jogo);
}

/* Desenha uma nota. Notas já acertadas não aparecem.
   Nota longa: um rastro da cabeça até o fim. Enquanto é segurada, a cabeça fica parada
   na linha de acerto e o rastro vai sendo consumido. */
void desenharNota(const Nota *nota, float tempo_atual){
    if(nota->estado == PERFEITO || nota->estado == BOM){
        return;
    }

    int nota_x = (int)calcularPosicaoX(nota->tempo, tempo_atual);
    int nota_y = calcularCentroYLane(nota->lane);
    int raio = TAMANHO_NOTA / 2;

    if(nota->estado == SEGURANDO){
        nota_x = POSICAO_X_LINHA_ACERTO;
    }

    Color cor_miolo = nota->injetada ? COR_NOTA_INJETADA : CORES_LANES[nota->lane];
    Color cor_borda = nota->injetada ? COR_BORDA_INJETADA : cor_miolo;

    if(nota->estado == ERRADO){
        cor_miolo = Fade(cor_miolo, OPACIDADE_NOTA_ERRADA);
        cor_borda = Fade(cor_borda, OPACIDADE_NOTA_ERRADA);
    }

    if(nota->duracao > 0.0f){
        int fim_x = (int)calcularPosicaoX(nota->tempo + nota->duracao, tempo_atual);
        if(fim_x > nota_x){
            DrawRectangle(nota_x, nota_y - ALTURA_RASTRO_NOTA / 2, fim_x - nota_x, ALTURA_RASTRO_NOTA, Fade(cor_miolo, OPACIDADE_RASTRO_NOTA));
        }
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
        DrawText(rotulos_lanes[lane], MARGEM_TELA, centro_y - TAMANHO_FONTE_INFO / 2, TAMANHO_FONTE_INFO, COR_TEXTO);
    }

    DrawRectangle(POSICAO_X_LINHA_ACERTO - ESPESSURA_LINHA_ACERTO / 2, POSICAO_Y_PAUTA,ESPESSURA_LINHA_ACERTO, ALTURA_PAUTA, COR_LINHA_ACERTO);

    /* Notas longas sendo seguradas: o cursor já passou delas, então são desenhadas à parte */
    for(int lane = 0; lane < QUANTIDADE_LANES; lane++){
        if(jogo->nota_segurada[lane] != NULL){
            desenharNota(jogo->nota_segurada[lane], jogo->tempo_atual);
        }
    }

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

    /* Respingos por cima das notas, recortados para não sujar a barra de informações */
    BeginScissorMode(0, POSICAO_Y_PAUTA, LARGURA_JANELA, ALTURA_PAUTA);
    for(int lane = 0; lane < QUANTIDADE_LANES; lane++){
        if(jogo->tempo_efeito_lane[lane] > 0.0f){
            desenharRespingoSangue(lane, jogo->tempo_efeito_lane[lane], jogo->resultado_efeito_lane[lane], jogo->semente_efeito_lane[lane]);
        }
    }
    EndScissorMode();
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
    desenharTextoCentralizado(texto, LARGURA_JANELA / 2, ALTURA_CENA / 4, TAMANHO_FONTE_FEEDBACK, Fade(cor, opacidade));
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
    desenharTextoCentralizado(titulo, centro_x, ALTURA_JANELA / 5, TAMANHO_FONTE_TITULO, venceu ? COR_PERFEITO : COR_TEXTO_DESTAQUE);

    float percentual = calcularPercentualAcertos(&jogo->lista);

    desenharTextoCentralizado(TextFormat("Pontuacao: %d", jogo->pontuacao), centro_x, ALTURA_JANELA / 2 - TAMANHO_FONTE_TEXTO, TAMANHO_FONTE_TEXTO, COR_TEXTO);
    desenharTextoCentralizado(TextFormat("Acertos: %.0f%% (minimo: %.0f%%)", percentual * 100.0f, PERCENTUAL_MINIMO_VITORIA * 100.0f), centro_x, ALTURA_JANELA / 2 + TAMANHO_FONTE_TEXTO,TAMANHO_FONTE_TEXTO, COR_TEXTO);
    desenharTextoCentralizado("ENTER para voltar ao menu", centro_x, ALTURA_JANELA * 4 / 5, TAMANHO_FONTE_TEXTO, COR_TEXTO);
}