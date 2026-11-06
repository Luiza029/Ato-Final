#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "raylib.h"
#include "gerador.h"
#include "listas_notas.h"
#include "ordenacao.h"
#include "mapa.h"
#include "melodia.h"

/*
 * Gerador automatico de mapas.
 *
 * As notas vem de dois detectores:
 * - Melodia (melodia.c): o comeco das notas de voz, piano, guitarra... A lane segue a
 *   altura da nota (grave = D, agudo = K) e a nota que continua soando vira nota longa.
 * - Batidas (este arquivo, explicado abaixo): bumbo, caixa e outros ataques fortes.
 *
 * Facil e medio usam a melodia (o facil so as notas mais marcadas). O dificil usa a melodia
 * e completa com as batidas onde nao ha nota da melodia por perto. Facil e medio tambem sao
 * completados com as batidas quando a melodia sozinha fica com poucas notas por segundo.
 * Se a musica quase nao tem melodia (so percussao), as tres dificuldades usam as batidas.
 *
 * Detector de batidas (energia):
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
    float sensibilidade;     // batidas: quantas vezes acima da media de energia
    float intervalo_minimo;  // segundos entre duas notas
    int permite_acordes;     // batidas: batida muito forte vira acorde
    float forca_minima;      // melodia: forca minima da nota (x forca de referencia da musica)
    int junta_batidas;       // melodia + batidas sempre (dificil)
    float densidade_minima;  // notas/s: se a melodia der menos que isso, as batidas completam
} ParametrosDificuldade;

/* Na mesma ordem do enum Dificuldade */
static const ParametrosDificuldade PARAMETROS_DIFICULDADES[QUANTIDADE_DIFICULDADES] = {
    { SENSIBILIDADE_FACIL,   INTERVALO_MINIMO_FACIL,   0, FORCA_MINIMA_FACIL,   0, DENSIDADE_MINIMA_FACIL },
    { SENSIBILIDADE_MEDIO,   INTERVALO_MINIMO_MEDIO,   0, FORCA_MINIMA_MEDIO,   0, DENSIDADE_MINIMA_MEDIO },
    { SENSIBILIDADE_DIFICIL, INTERVALO_MINIMO_DIFICIL, 1, FORCA_MINIMA_DIFICIL, 1, 0.0f }
};

/* Nota candidata da melodia: a forca dela e a posicao no vetor de notas. */
typedef struct CandidataMelodia{
    float forca;
    int indice;
} CandidataMelodia;

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

/* Comparadores para o qsort. */
static int compararForcaDecrescente(const void *primeira, const void *segunda){
    float forca_primeira = ((const CandidataMelodia *)primeira)->forca;
    float forca_segunda = ((const CandidataMelodia *)segunda)->forca;
    return (forca_primeira < forca_segunda) - (forca_primeira > forca_segunda);
}

static int compararFloatCrescente(const void *primeiro, const void *segundo){
    float valor_primeiro = *(const float *)primeiro;
    float valor_segundo = *(const float *)segundo;
    return (valor_primeiro > valor_segundo) - (valor_primeiro < valor_segundo);
}

static int compararIntCrescente(const void *primeiro, const void *segundo){
    return *(const int *)primeiro - *(const int *)segundo;
}

/* Escolhe as notas da melodia de uma dificuldade, das mais fortes para as mais fracas:
   pula as fracas demais (abaixo de forca_minima x forca de referencia) e as que ficariam a
   menos de intervalo_minimo de uma nota ja escolhida. Assim, quando duas notas estao perto,
   fica a mais marcada. Escreve em escolhidas os indices (em ordem de tempo) e retorna quantos. */
static int selecionarNotasMelodia(const NotaMelodia *notas, int quantidade, ParametrosDificuldade parametros, int *escolhidas){
    CandidataMelodia *candidatas = malloc(quantidade * sizeof(CandidataMelodia));
    float *forcas = malloc(quantidade * sizeof(float));

    if(candidatas == NULL || forcas == NULL){
        free(candidatas);
        free(forcas);
        return 0;
    }

    for(int i = 0; i < quantidade; i++){
        candidatas[i].forca = notas[i].forca;
        candidatas[i].indice = i;
        forcas[i] = notas[i].forca;
    }

    // Forca de referencia: a nota que fica entre as 10% mais marcadas. A mediana nao serve,
    // porque os restos de batida que escapam sao muitos e fracos e puxariam a mediana para baixo.
    qsort(forcas, quantidade, sizeof(float), compararFloatCrescente);
    int posicao_referencia = (int)(quantidade * PERCENTIL_REFERENCIA_FORCA);
    if(posicao_referencia >= quantidade){
        posicao_referencia = quantidade - 1;
    }
    float forca_minima = forcas[posicao_referencia] * parametros.forca_minima;

    qsort(candidatas, quantidade, sizeof(CandidataMelodia), compararForcaDecrescente);

    int quantidade_escolhidas = 0;

    for(int c = 0; c < quantidade && candidatas[c].forca >= forca_minima; c++){
        float tempo = notas[candidatas[c].indice].tempo;
        int muito_perto = 0;

        for(int e = 0; e < quantidade_escolhidas && !muito_perto; e++){
            if(fabsf(notas[escolhidas[e]].tempo - tempo) < parametros.intervalo_minimo){
                muito_perto = 1;
            }
        }

        if(!muito_perto){
            escolhidas[quantidade_escolhidas++] = candidatas[c].indice;
        }
    }

    // As notas do vetor ja estao em ordem de tempo, entao ordenar os indices basta
    qsort(escolhidas, quantidade_escolhidas, sizeof(int), compararIntCrescente);

    // Nota fraca que cai dentro de uma nota longa bem mais forte e "engolida" por ela:
    // quase sempre e acompanhamento ou resto de batida, e cortaria a nota longa no meio.
    int quantidade_mantidas = 0;
    float fim_nota_longa = -1.0f;
    float forca_nota_longa = 0.0f;

    for(int e = 0; e < quantidade_escolhidas; e++){
        const NotaMelodia *nota = &notas[escolhidas[e]];

        // A folga cobre o "rabo" do som: a nota seguinte pode comecar logo antes da longa sumir de vez
        if(nota->tempo < fim_nota_longa - FOLGA_ANTES_PROXIMA_NOTA && nota->forca < forca_nota_longa * FRACAO_FORCA_ENGOLIDA){
            continue;
        }

        escolhidas[quantidade_mantidas++] = escolhidas[e];

        if(nota->sustentacao >= DURACAO_MINIMA_NOTA_LONGA){
            fim_nota_longa = nota->tempo + nota->sustentacao;
            forca_nota_longa = nota->forca;
        }else{
            fim_nota_longa = -1.0f;
        }
    }

    free(candidatas);
    free(forcas);
    return quantidade_mantidas;
}

/* Insere na lista as notas escolhidas da melodia.
   Lane pela altura: as alturas desta dificuldade sao divididas em QUANTIDADE_LANES grupos do
   mesmo tamanho, do mais grave (D) ao mais agudo (K). Assim o mapa sobe e desce junto com a
   melodia. Nota repetida (mesma altura da anterior) fica na mesma lane.
   A nota que continua soando vira nota longa, sem encostar na proxima nota. */
static void inserirNotasMelodia(const NotaMelodia *notas, const int *escolhidas, int quantidade_escolhidas, Listas_notas *lista){
    float *alturas = malloc(quantidade_escolhidas * sizeof(float));

    if(alturas == NULL){
        return;
    }

    for(int e = 0; e < quantidade_escolhidas; e++){
        alturas[e] = notas[escolhidas[e]].altura;
    }

    qsort(alturas, quantidade_escolhidas, sizeof(float), compararFloatCrescente);

    float limites[QUANTIDADE_LANES - 1];
    for(int lane = 1; lane < QUANTIDADE_LANES; lane++){
        limites[lane - 1] = alturas[quantidade_escolhidas * lane / QUANTIDADE_LANES];
    }

    int lane_anterior = -1;
    float altura_anterior = 0.0f;

    for(int e = 0; e < quantidade_escolhidas; e++){
        const NotaMelodia *nota = &notas[escolhidas[e]];

        int lane = 0;
        while(lane < QUANTIDADE_LANES - 1 && nota->altura >= limites[lane]){
            lane++;
        }

        if(lane_anterior >= 0 && fabsf(nota->altura - altura_anterior) <= TOLERANCIA_MESMA_ALTURA){
            lane = lane_anterior;
        }

        float duracao = nota->sustentacao;
        if(duracao > DURACAO_MAXIMA_NOTA_LONGA){
            duracao = DURACAO_MAXIMA_NOTA_LONGA;
        }
        if(e + 1 < quantidade_escolhidas){
            float ate_proxima = notas[escolhidas[e + 1]].tempo - nota->tempo - FOLGA_ANTES_PROXIMA_NOTA;
            if(duracao > ate_proxima){
                duracao = ate_proxima;
            }
        }
        if(duracao < DURACAO_MINIMA_NOTA_LONGA){
            duracao = 0.0f;
        }

        inserirNota(lista, nota->tempo, lane, 0);
        lista->fim->duracao = duracao;

        lane_anterior = lane;
        altura_anterior = nota->altura;
    }

    free(alturas);
}

/* As batidas (bumbo, caixa...) entram junto com as notas da melodia que ja estao na lista,
   mas so onde nao atrapalham: longe (intervalo_minimo) do comeco de qualquer nota da melodia
   e fora das notas longas, que deixam a mao ocupada ate o fim. */
static void juntarBatidas(const Listas_notas *batidas, float intervalo_minimo, Listas_notas *lista){
    // As batidas sao inseridas no fim da lista: so as notas ate aqui sao da melodia
    const Nota *ultima_da_melodia = lista->fim;

    for(const Nota *batida = batidas->inicio; batida != NULL; batida = batida->proximo){
        int atrapalha = 0;

        for(const Nota *nota = lista->inicio; nota != NULL; nota = nota->proximo){
            int perto_do_comeco = fabsf(nota->tempo - batida->tempo) < intervalo_minimo;
            int dentro_da_longa = nota->duracao > 0.0f && batida->tempo > nota->tempo &&
                                  batida->tempo < nota->tempo + nota->duracao + FOLGA_ANTES_PROXIMA_NOTA;

            if(perto_do_comeco || dentro_da_longa){
                atrapalha = 1;
            }

            if(atrapalha || nota == ultima_da_melodia){
                break;
            }
        }

        if(!atrapalha){
            inserirNota(lista, batida->tempo, batida->lane, 0);
        }
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

    // Notas da melodia. Se a musica quase nao tem melodia (so percussao), fica so com as batidas.
    // A melodia vai so ate FREQUENCIA_MAXIMA_MELODIA, entao ela e analisada numa copia do som com
    // taxa menor (WaveFormat ja filtra os agudos ao reduzir a taxa): 4x menos amostras e FFT 4x menor.
    NotaMelodia *notas_melodia = NULL;
    int quantidade_melodia = -1;
    Wave onda_melodia = WaveCopy(onda);
    WaveFormat(&onda_melodia, TAXA_AMOSTRAGEM_MELODIA, 32, 1);
    float *amostras_melodia = LoadWaveSamples(onda_melodia);

    if(amostras_melodia != NULL){
        quantidade_melodia = detectarNotasMelodia(amostras_melodia, (int)onda_melodia.frameCount, TAXA_AMOSTRAGEM_MELODIA, &notas_melodia);
    }

    UnloadWaveSamples(amostras_melodia);
    UnloadWave(onda_melodia);
    int *escolhidas = (quantidade_melodia > 0) ? malloc(quantidade_melodia * sizeof(int)) : NULL;
    int usa_melodia = quantidade_melodia >= MINIMO_NOTAS_MELODIA && escolhidas != NULL;
    float duracao_musica = (float)onda.frameCount / onda.sampleRate;

    int deu_certo = 1;

    for(int dificuldade = 0; dificuldade < QUANTIDADE_DIFICULDADES; dificuldade++){
        Listas_notas lista;
        criarLista(&lista);

        ParametrosDificuldade parametros = PARAMETROS_DIFICULDADES[dificuldade];
        int quantidade_escolhidas = 0;

        if(usa_melodia){
            quantidade_escolhidas = selecionarNotasMelodia(notas_melodia, quantidade_melodia, parametros, escolhidas);
        }

        if(quantidade_escolhidas > 0){
            inserirNotasMelodia(notas_melodia, escolhidas, quantidade_escolhidas, &lista);

            // Melodia com poucas notas (ex.: notas repetidas na mesma altura, que quase nao
            // mudam o espectro): as batidas completam o mapa
            float densidade_melodia = quantidade_escolhidas / duracao_musica;

            if(parametros.junta_batidas || densidade_melodia < parametros.densidade_minima){
                Listas_notas batidas;
                criarLista(&batidas);
                detectarBatidas(energias, cruzamentos, quantidade_janelas, media_cruzamentos, segundos_por_passo,
                                janelas_historico, passos_por_janela, parametros, &batidas);
                juntarBatidas(&batidas, parametros.intervalo_minimo, &lista);
                liberarLista(&batidas);
            }
        }else{
            detectarBatidas(energias, cruzamentos, quantidade_janelas, media_cruzamentos, segundos_por_passo,
                            janelas_historico, passos_por_janela, parametros, &lista);
        }

        // As batidas do dificil entraram depois da melodia: ordena antes de alinhar (o alinhamento
        // compara cada nota com a anterior) e de novo depois, porque alinhar pode trocar duas de lugar.
        // Com a lista quase ordenada, o Insertion Sort faz isso em quase uma passada so.
        ordenarNotas(&lista);
        alinharNotasNaBatida(&lista, periodo_batida);
        ordenarNotas(&lista);

        char caminho_mapa[TAMANHO_MAXIMO_CAMINHO];
        montarCaminhoMapa(caminho_mapa, TAMANHO_MAXIMO_CAMINHO, caminho_musica, (Dificuldade)dificuldade);

        if(lista.quantidade_de_notas == 0 || !salvarMapa(&lista, caminho_mapa)){
            deu_certo = 0;
        }

        liberarLista(&lista);
    }

    free(notas_melodia);
    free(escolhidas);
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
