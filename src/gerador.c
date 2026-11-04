#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "gerador.h"
#include "listas_notas.h"
#include "ordenacao.h"
#include "mapa.h"

/*
 * Gerador automatico de mapas por deteccao de batidas (energia).
 *
 * 1. A musica e convertida em amostras mono (numeros de -1 a 1).
 * 2. As amostras sao cortadas em janelas de AMOSTRAS_POR_JANELA_GERADOR.
 *    Para cada janela calculamos a energia (media dos quadrados) e a taxa de
 *    cruzamentos por zero (quantas vezes o som troca de sinal: som grave troca
 *    pouco, som agudo troca muito).
 * 3. Uma janela vira nota quando a energia dela passa da media do ultimo segundo
 *    multiplicada pela sensibilidade da dificuldade, deu um salto em relacao a
 *    janela anterior e respeita o intervalo minimo entre notas.
 * 4. Sons graves (bumbo, baixo) vao para D/F e sons agudos (caixa, prato) para J/K.
 *    Dentro de cada par as lanes se alternam para o padrao nao ficar repetitivo.
 * 5. As notas entram na lista, a lista e ordenada e salva no .txt do mapa.
 */

typedef struct ParametrosDificuldade{
    float sensibilidade;
    float intervalo_minimo;
    int permite_acordes;
} ParametrosDificuldade;

/* Na mesma ordem do enum Dificuldade */
static const ParametrosDificuldade PARAMETROS_DIFICULDADES[QUANTIDADE_DIFICULDADES] = {
    { SENSIBILIDADE_FACIL,   INTERVALO_MINIMO_FACIL,   0 },
    { SENSIBILIDADE_MEDIO,   INTERVALO_MINIMO_MEDIO,   0 },
    { SENSIBILIDADE_DIFICIL, INTERVALO_MINIMO_DIFICIL, 1 }
};

static const char *SUFIXOS_DIFICULDADES[QUANTIDADE_DIFICULDADES] = {
    "facil",
    "medio",
    "dificil"
};

/* Energia de uma janela: media dos quadrados das amostras. */
static float calcularEnergia(const float *amostras, int quantidade){
    float soma = 0.0f;

    for(int i = 0; i < quantidade; i++){
        soma += amostras[i] * amostras[i];
    }

    return soma / quantidade;
}

/* Fracao das amostras em que o som troca de sinal (de 0 a 1). */
static float calcularCruzamentosZero(const float *amostras, int quantidade){
    int cruzamentos = 0;

    for(int i = 1; i < quantidade; i++){
        if((amostras[i - 1] < 0.0f) != (amostras[i] < 0.0f)){
            cruzamentos++;
        }
    }

    return (float)cruzamentos / quantidade;
}

/* Escolhe a lane pela "altura" do som e alterna dentro do par (D/F ou J/K). */
static int escolherLane(float cruzamentos, float media_cruzamentos, int alternancia_par[2]){
    int par = (cruzamentos < media_cruzamentos) ? 0 : 1;   // 0 = grave, 1 = agudo

    int lane = par * 2 + alternancia_par[par];
    alternancia_par[par] = 1 - alternancia_par[par];

    return lane;
}

/* Percorre as janelas e insere na lista as notas de uma dificuldade. */
static void detectarBatidas(const float *energias, const float *cruzamentos, int quantidade_janelas,
                            float media_cruzamentos, float segundos_por_janela,
                            ParametrosDificuldade parametros, Listas_notas *lista){
    float soma_historico = 0.0f;
    float tempo_ultima_nota = -parametros.intervalo_minimo;
    int alternancia_par[2] = { 0, 0 };
    int ultima_foi_acorde = 0;

    for(int janela = 0; janela < quantidade_janelas; janela++){
        if(janela >= JANELAS_HISTORICO_GERADOR){
            float media_energia = soma_historico / JANELAS_HISTORICO_GERADOR;
            float energia = energias[janela];
            float tempo = janela * segundos_por_janela;

            // Batida e um salto de energia: som que so cresce devagar (um fundo aumentando) nao conta
            float aumento = energia - energias[janela - 1];

            int e_batida = energia > media_energia * parametros.sensibilidade &&
                           aumento > media_energia * (parametros.sensibilidade - 1.0f) &&
                           energia > ENERGIA_MINIMA_BATIDA &&
                           tempo - tempo_ultima_nota >= parametros.intervalo_minimo;

            if(e_batida){
                int lane = escolherLane(cruzamentos[janela], media_cruzamentos, alternancia_par);
                inserirNota(lista, tempo, lane, 0);

                // No dificil, batida muito forte vira acorde com a lane do outro par (nunca dois acordes seguidos)
                int vira_acorde = parametros.permite_acordes && !ultima_foi_acorde &&
                                  energia > media_energia * FATOR_ACORDE_DIFICIL;

                if(vira_acorde){
                    inserirNota(lista, tempo, (lane + 2) % QUANTIDADE_LANES, 0);
                }

                ultima_foi_acorde = vira_acorde;

                tempo_ultima_nota = tempo;
            }

            // A janela mais antiga sai da media do ultimo segundo
            soma_historico -= energias[janela - JANELAS_HISTORICO_GERADOR];
        }

        soma_historico += energias[janela];
    }
}

void montarCaminhoMapa(char *destino, int tamanho_destino, const char *caminho_musica, Dificuldade dificuldade){
    snprintf(destino, tamanho_destino, "%s%s_%s.txt",
             PASTA_MAPAS, GetFileNameWithoutExt(caminho_musica), SUFIXOS_DIFICULDADES[dificuldade]);
}

int gerarMapas(const char *caminho_musica){
    Wave onda = LoadWave(caminho_musica);

    if(!IsWaveValid(onda)){
        return 0;
    }

    // Deixa o som em mono e 32 bits para cada amostra virar um float de -1 a 1
    WaveFormat(&onda, onda.sampleRate, 32, 1);
    float *amostras = LoadWaveSamples(onda);
    int quantidade_janelas = (int)(onda.frameCount / AMOSTRAS_POR_JANELA_GERADOR);
    float segundos_por_janela = (float)AMOSTRAS_POR_JANELA_GERADOR / onda.sampleRate;

    float *energias = malloc(quantidade_janelas * sizeof(float));
    float *cruzamentos = malloc(quantidade_janelas * sizeof(float));

    if(amostras == NULL || energias == NULL || cruzamentos == NULL || quantidade_janelas <= JANELAS_HISTORICO_GERADOR){
        free(energias);
        free(cruzamentos);
        UnloadWaveSamples(amostras);
        UnloadWave(onda);
        return 0;
    }

    // Media dos cruzamentos so das janelas com som: separa grave de agudo nesta musica
    float soma_cruzamentos = 0.0f;
    int janelas_com_som = 0;

    for(int janela = 0; janela < quantidade_janelas; janela++){
        const float *inicio = amostras + janela * AMOSTRAS_POR_JANELA_GERADOR;
        energias[janela] = calcularEnergia(inicio, AMOSTRAS_POR_JANELA_GERADOR);
        cruzamentos[janela] = calcularCruzamentosZero(inicio, AMOSTRAS_POR_JANELA_GERADOR);

        if(energias[janela] > ENERGIA_MINIMA_BATIDA){
            soma_cruzamentos += cruzamentos[janela];
            janelas_com_som++;
        }
    }

    float media_cruzamentos = (janelas_com_som > 0) ? soma_cruzamentos / janelas_com_som : 0.0f;

    int deu_certo = 1;

    for(int dificuldade = 0; dificuldade < QUANTIDADE_DIFICULDADES; dificuldade++){
        Listas_notas lista;
        criarLista(&lista);

        detectarBatidas(energias, cruzamentos, quantidade_janelas, media_cruzamentos,
                        segundos_por_janela, PARAMETROS_DIFICULDADES[dificuldade], &lista);
        ordenarNotas(&lista);

        char caminho_mapa[TAMANHO_MAXIMO_CAMINHO];
        montarCaminhoMapa(caminho_mapa, TAMANHO_MAXIMO_CAMINHO, caminho_musica, (Dificuldade)dificuldade);

        if(lista.quantidade_de_notas == 0 || !salvarMapa(&lista, caminho_mapa)){
            deu_certo = 0;
        }

        liberarLista(&lista);
    }

    free(energias);
    free(cruzamentos);
    UnloadWaveSamples(amostras);
    UnloadWave(onda);

    return deu_certo;
}

int prepararMapasDaMusica(const char *caminho_musica, char caminhos_mapas[QUANTIDADE_DIFICULDADES][TAMANHO_MAXIMO_CAMINHO]){
    char caminhos_novos[QUANTIDADE_DIFICULDADES][TAMANHO_MAXIMO_CAMINHO];
    int todos_existem = 1;

    for(int dificuldade = 0; dificuldade < QUANTIDADE_DIFICULDADES; dificuldade++){
        montarCaminhoMapa(caminhos_novos[dificuldade], TAMANHO_MAXIMO_CAMINHO, caminho_musica, (Dificuldade)dificuldade);

        if(!FileExists(caminhos_novos[dificuldade])){
            todos_existem = 0;
        }
    }

    if(!todos_existem && !gerarMapas(caminho_musica)){
        return 0;
    }

    for(int dificuldade = 0; dificuldade < QUANTIDADE_DIFICULDADES; dificuldade++){
        TextCopy(caminhos_mapas[dificuldade], caminhos_novos[dificuldade]);
    }

    return 1;
}
