#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "raylib.h"
#include "gerador.h"
#include "listas_notas.h"
#include "ordenacao.h"
#include "mapa.h"

/*
 * Gerador automatico de mapas por deteccao de batidas (energia).
 *
 * 1. A musica e convertida em amostras mono (numeros de -1 a 1).
 * 2. As amostras sao cortadas em janelas de AMOSTRAS_POR_JANELA_GERADOR que andam
 *    de AMOSTRAS_PASSO_GERADOR em AMOSTRAS_PASSO_GERADOR (uma janela sobrepoe a outra,
 *    entao a nota pode cair a cada ~5 ms, e nao so a cada ~21 ms).
 *    Para cada janela calculamos a energia (media dos quadrados) e a taxa de
 *    cruzamentos por zero (quantas vezes o som troca de sinal: som grave troca
 *    pouco, som agudo troca muito).
 * 3. Uma janela vira nota quando a energia dela passa da media do ultimo segundo
 *    multiplicada pela sensibilidade da dificuldade, deu um salto em relacao a
 *    janela de um tamanho atras e respeita o intervalo minimo entre notas.
 * 4. Sons graves (bumbo, baixo) vao para D/F e sons agudos (caixa, prato) para J/K.
 *    Dentro de cada par as lanes se alternam para o padrao nao ficar repetitivo.
 * 5. Alinhamento na batida: o gerador descobre de quanto em quanto tempo a musica
 *    "bate" e puxa cada nota para a batida (ou meia batida) mais proxima. Assim o
 *    espaco entre as notas fica constante. Nota longe de qualquer batida fica onde esta.
 * 6. As notas entram na lista, a lista e ordenada e salva no .txt do mapa.
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

/* Percorre as janelas e insere na lista as notas de uma dificuldade.
   O salto de energia e medido contra a janela de um tamanho atras (passos_por_janela),
   que nao se sobrepoe com a atual. */
static void detectarBatidas(const float *energias, const float *cruzamentos, int quantidade_janelas,
                            float media_cruzamentos, float segundos_por_passo, int janelas_historico,
                            int passos_por_janela, ParametrosDificuldade parametros, Listas_notas *lista){
    float soma_historico = 0.0f;
    float tempo_ultima_nota = -parametros.intervalo_minimo;
    int alternancia_par[2] = { 0, 0 };
    int ultima_foi_acorde = 0;

    for(int janela = 0; janela < quantidade_janelas; janela++){
        if(janela >= janelas_historico){
            float media_energia = soma_historico / janelas_historico;
            float energia = energias[janela];
            float tempo = janela * segundos_por_passo;

            // Batida e um salto de energia: som que so cresce devagar (um fundo aumentando) nao conta
            float aumento = energia - energias[janela - passos_por_janela];

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
            soma_historico -= energias[janela - janelas_historico];
        }

        soma_historico += energias[janela];
    }
}

/* Estima de quanto em quanto tempo a musica bate (autocorrelacao).
   Pega so os aumentos de energia (os "ataques" do som) e testa cada atraso possivel
   entre PERIODO_MINIMO_BATIDA e PERIODO_MAXIMO_BATIDA: o atraso em que os ataques
   mais se repetem e o periodo da batida. Retorna 0 se nao conseguir estimar. */
static float estimarPeriodoBatida(const float *energias, int quantidade_janelas,
                                  float segundos_por_passo, int passos_por_janela){
    int menor_atraso = (int)(PERIODO_MINIMO_BATIDA / segundos_por_passo);
    int maior_atraso = (int)(PERIODO_MAXIMO_BATIDA / segundos_por_passo);
    double melhor_semelhanca = 0.0;
    int melhor_atraso = 0;

    for(int atraso = menor_atraso; atraso <= maior_atraso; atraso++){
        double semelhanca = 0.0;
        int comparacoes = 0;

        for(int janela = passos_por_janela + atraso; janela < quantidade_janelas; janela++){
            float ataque_agora = energias[janela] - energias[janela - passos_por_janela];
            float ataque_antes = energias[janela - atraso] - energias[janela - atraso - passos_por_janela];

            if(ataque_agora > 0.0f && ataque_antes > 0.0f){
                semelhanca += (double)ataque_agora * ataque_antes;
            }
            comparacoes++;
        }

        if(comparacoes > 0){
            semelhanca /= comparacoes;
        }

        if(semelhanca > melhor_semelhanca){
            melhor_semelhanca = semelhanca;
            melhor_atraso = atraso;
        }
    }

    return melhor_atraso * segundos_por_passo;
}

/* Puxa as notas para a batida (ou meia batida) mais proxima.
   1. Refina o periodo: testa periodos perto do estimado e fica com aquele em que as notas
      mais "concordam" (cada nota vira um angulo no relogio da batida; se as notas estao no
      ritmo, os angulos apontam para o mesmo lado e a soma deles fica grande).
   2. Esse angulo medio diz onde a grade comeca (a fase).
   3. Cada nota a ate TOLERANCIA_GRADE de um ponto da grade vai para esse ponto.
      Se duas notas diferentes caissem no mesmo ponto, a segunda fica onde estava. */
static void alinharNotasNaBatida(Listas_notas *lista, float periodo_estimado){
    if(periodo_estimado <= 0.0f || lista->quantidade_de_notas < MINIMO_NOTAS_PARA_GRADE){
        return;
    }

    double melhor_concordancia = -1.0;
    double melhor_grade = 0.0;
    double melhor_angulo = 0.0;

    for(int tentativa = 0; tentativa <= PASSOS_BUSCA_PERIODO; tentativa++){
        double fator = 1.0 - VARIACAO_BUSCA_PERIODO + 2.0 * VARIACAO_BUSCA_PERIODO * tentativa / PASSOS_BUSCA_PERIODO;
        double grade = periodo_estimado * fator / DIVISOES_POR_BATIDA;
        double soma_cosseno = 0.0;
        double soma_seno = 0.0;

        for(Nota *nota = lista->inicio; nota != NULL; nota = nota->proximo){
            double angulo = 2.0 * PI * nota->tempo / grade;
            soma_cosseno += cos(angulo);
            soma_seno += sin(angulo);
        }

        double concordancia = soma_cosseno * soma_cosseno + soma_seno * soma_seno;
        if(concordancia > melhor_concordancia){
            melhor_concordancia = concordancia;
            melhor_grade = grade;
            melhor_angulo = atan2(soma_seno, soma_cosseno);
        }
    }

    double inicio_grade = melhor_angulo / (2.0 * PI) * melhor_grade;
    float tempo_original_anterior = -1.0f;
    float tempo_alinhado_anterior = -1.0f;

    for(Nota *nota = lista->inicio; nota != NULL; nota = nota->proximo){
        float tempo_original = nota->tempo;
        float tempo_na_grade = (float)(round((tempo_original - inicio_grade) / melhor_grade) * melhor_grade + inicio_grade);

        // Notas de um acorde (mesmo tempo) continuam juntas; notas diferentes nao podem virar uma so
        int colide_com_anterior = tempo_na_grade == tempo_alinhado_anterior && tempo_original != tempo_original_anterior;

        if(fabsf(tempo_original - tempo_na_grade) <= TOLERANCIA_GRADE && tempo_na_grade >= 0.0f && !colide_com_anterior){
            nota->tempo = tempo_na_grade;
        }

        tempo_original_anterior = tempo_original;
        tempo_alinhado_anterior = nota->tempo;
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
    int quantidade_janelas = 0;
    if(onda.frameCount >= AMOSTRAS_POR_JANELA_GERADOR){
        quantidade_janelas = (int)((onda.frameCount - AMOSTRAS_POR_JANELA_GERADOR) / AMOSTRAS_PASSO_GERADOR) + 1;
    }
    float segundos_por_passo = (float)AMOSTRAS_PASSO_GERADOR / onda.sampleRate;
    int janelas_historico = (int)(SEGUNDOS_HISTORICO_GERADOR / segundos_por_passo);
    int passos_por_janela = AMOSTRAS_POR_JANELA_GERADOR / AMOSTRAS_PASSO_GERADOR;

    float *energias = malloc(quantidade_janelas * sizeof(float));
    float *cruzamentos = malloc(quantidade_janelas * sizeof(float));

    if(amostras == NULL || energias == NULL || cruzamentos == NULL || quantidade_janelas <= janelas_historico){
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
        const float *inicio = amostras + janela * AMOSTRAS_PASSO_GERADOR;
        energias[janela] = calcularEnergia(inicio, AMOSTRAS_POR_JANELA_GERADOR);
        cruzamentos[janela] = calcularCruzamentosZero(inicio, AMOSTRAS_POR_JANELA_GERADOR);

        if(energias[janela] > ENERGIA_MINIMA_BATIDA){
            soma_cruzamentos += cruzamentos[janela];
            janelas_com_som++;
        }
    }

    float media_cruzamentos = (janelas_com_som > 0) ? soma_cruzamentos / janelas_com_som : 0.0f;
    float periodo_batida = estimarPeriodoBatida(energias, quantidade_janelas, segundos_por_passo, passos_por_janela);

    int deu_certo = 1;

    for(int dificuldade = 0; dificuldade < QUANTIDADE_DIFICULDADES; dificuldade++){
        Listas_notas lista;
        criarLista(&lista);

        detectarBatidas(energias, cruzamentos, quantidade_janelas, media_cruzamentos, segundos_por_passo,
                        janelas_historico, passos_por_janela, PARAMETROS_DIFICULDADES[dificuldade], &lista);
        alinharNotasNaBatida(&lista, periodo_batida);
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
