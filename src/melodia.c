#include <stdlib.h>
#include <math.h>
#include "melodia.h"
#include "constantes.h"

/*
 * Detector de notas da melodia por fluxo espectral.
 *
 * O detector de batidas (gerador.c) olha so a energia total do som, e quase toda a
 * energia de uma musica esta nos graves (bumbo, baixo). Por isso ele segue o fundo.
 * Aqui olhamos cada faixa de frequencia separada:
 *
 * 1. O som e cortado em pedacos de TAMANHO_FFT_MELODIA amostras, que andam de
 *    PASSO_FFT_MELODIA em PASSO_FFT_MELODIA. Cada pedaco e suavizado nas pontas
 *    (janela de Hann) para nao criar frequencias falsas no corte.
 * 2. A FFT (transformada rapida de Fourier) separa o pedaco em faixas de frequencia,
 *    como um equalizador com centenas de barrinhas. Ficamos so com as faixas entre
 *    FREQUENCIA_MINIMA_MELODIA e FREQUENCIA_MAXIMA_MELODIA (voz, piano, guitarra...).
 * 2b. Separacao nota/batida (HPSS): o bumbo e a caixa tambem acendem essas faixas por
 *    um instante. Um filtro de mediana separa o que e nota (som que dura) do que e
 *    batida (som que acende tudo de uma vez) e ficamos so com as notas.
 * 3. Fluxo espectral: para cada pedaco, somamos quanto cada faixa AUMENTOU em relacao
 *    ao pedaco anterior. Quando uma nota nova comeca, as faixas da altura dela "acendem",
 *    mesmo que o volume total da musica nao mude.
 * 4. Cada pico do fluxo que passa bem acima da media dos vizinhos e o comeco de uma nota.
 * 5. A faixa que mais cresceu nesse pico diz a altura da nota, e acompanhando essa
 *    faixa ate ela enfraquecer descobrimos por quanto tempo a nota continua soando.
 */

/* FFT iterativa (Cooley-Tukey, raiz 2). Troca o sinal (real, imaginario) pelo seu
   espectro, no proprio vetor. n precisa ser potencia de 2.
   cossenos[k] e senos[k] guardam cos e sin de -2*pi*k/n (k de 0 a n/2), calculados uma vez
   so para todos os pedacos da musica: as "borboletas" so consultam a tabela. */
static void calcularFFT(float *real, float *imaginario, int n, const float *cossenos, const float *senos){
    // 1. Reordena as posicoes pelo "inverso dos bits" do indice (ex.: 001 <-> 100)
    for(int i = 1, j = 0; i < n; i++){
        int bit = n >> 1;
        while(j & bit){
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;

        if(i < j){
            float troca = real[i];
            real[i] = real[j];
            real[j] = troca;

            troca = imaginario[i];
            imaginario[i] = imaginario[j];
            imaginario[j] = troca;
        }
    }

    // 2. Junta as transformadas de 2 em 2, de 4 em 4, ... ate n ("borboletas")
    for(int tamanho = 2; tamanho <= n; tamanho <<= 1){
        int metade = tamanho / 2;
        int passo_tabela = n / tamanho;

        for(int inicio = 0; inicio < n; inicio += tamanho){
            for(int k = 0; k < metade; k++){
                int a = inicio + k;
                int b = a + metade;
                float fator_real = cossenos[k * passo_tabela];
                float fator_imaginario = senos[k * passo_tabela];

                float produto_real = real[b] * fator_real - imaginario[b] * fator_imaginario;
                float produto_imaginario = real[b] * fator_imaginario + imaginario[b] * fator_real;

                real[b] = real[a] - produto_real;
                imaginario[b] = imaginario[a] - produto_imaginario;
                real[a] += produto_real;
                imaginario[a] += produto_imaginario;
            }
        }
    }
}

/* Mediana de poucos valores: ordena com Insertion Sort (vetor pequeno) e pega o do meio. */
static float calcularMediana(float *valores, int quantidade){
    for(int i = 1; i < quantidade; i++){
        float atual = valores[i];
        int j = i - 1;

        while(j >= 0 && valores[j] > atual){
            valores[j + 1] = valores[j];
            j--;
        }

        valores[j + 1] = atual;
    }

    return valores[quantidade / 2];
}

/* Separacao harmonico-percussiva (HPSS, filtro de mediana).
   No espectro, uma nota tocada e uma linha HORIZONTAL: a mesma faixa fica acesa por varios
   pedacos seguidos. Uma batida (bumbo, caixa) e uma linha VERTICAL: acende quase todas as
   faixas, mas so por um instante.
   - A mediana no tempo (mesma faixa, pedacos vizinhos) mantem as linhas horizontais e apaga as verticais.
   - A mediana na frequencia (mesmo pedaco, faixas vizinhas) faz o contrario.
   Cada ponto fica com a fracao "nota" do seu valor: harmonico^2 / (harmonico^2 + percussivo^2).
   Retorna um espectro novo (alocado com malloc) ou NULL se faltou memoria. */
static float *separarHarmonico(const float *espectro, int quantidade_analises, int quantidade_faixas){
    float *harmonico = malloc((size_t)quantidade_analises * quantidade_faixas * sizeof(float));
    float vizinhos_tempo[2 * RAIO_MEDIANA_TEMPO + 1];
    float vizinhos_frequencia[2 * RAIO_MEDIANA_FREQUENCIA + 1];

    if(harmonico == NULL){
        return NULL;
    }

    for(int analise = 0; analise < quantidade_analises; analise++){
        for(int faixa = 0; faixa < quantidade_faixas; faixa++){
            int quantidade = 0;
            for(int vizinho = analise - RAIO_MEDIANA_TEMPO; vizinho <= analise + RAIO_MEDIANA_TEMPO; vizinho++){
                if(vizinho >= 0 && vizinho < quantidade_analises){
                    vizinhos_tempo[quantidade++] = espectro[(size_t)vizinho * quantidade_faixas + faixa];
                }
            }
            float parte_nota = calcularMediana(vizinhos_tempo, quantidade);

            quantidade = 0;
            for(int vizinha = faixa - RAIO_MEDIANA_FREQUENCIA; vizinha <= faixa + RAIO_MEDIANA_FREQUENCIA; vizinha++){
                if(vizinha >= 0 && vizinha < quantidade_faixas){
                    vizinhos_frequencia[quantidade++] = espectro[(size_t)analise * quantidade_faixas + vizinha];
                }
            }
            float parte_batida = calcularMediana(vizinhos_frequencia, quantidade);

            float soma = parte_nota * parte_nota + parte_batida * parte_batida;
            float fracao_nota = (soma > 0.0f) ? parte_nota * parte_nota / soma : 0.0f;

            harmonico[(size_t)analise * quantidade_faixas + faixa] = espectro[(size_t)analise * quantidade_faixas + faixa] * fracao_nota;
        }
    }

    return harmonico;
}

/* Quanto a faixa cresceu no ataque. Olha tambem as faixas vizinhas, porque o harmonico
   quase nunca cai exatamente no meio de uma faixa. Fora da faixa da melodia conta 0. */
static float aumentoPerto(const float *faixas_antes, const float *faixas_depois, int quantidade_faixas, int faixa){
    float maior = 0.0f;

    for(int vizinha = faixa - 1; vizinha <= faixa + 1; vizinha++){
        if(vizinha >= 0 && vizinha < quantidade_faixas){
            float aumento = faixas_depois[vizinha] - faixas_antes[vizinha];
            if(aumento > maior){
                maior = aumento;
            }
        }
    }

    return maior;
}

/* Escolhe a faixa da nota fundamental (soma de harmonicos).
   Uma nota de frequencia F tambem soa em 2F, 3F... (os harmonicos). Olhar so a faixa que
   mais cresceu as vezes pega 2F e a nota sai uma oitava acima. Por isso cada faixa ganha
   pontos pelo proprio aumento e pelos aumentos em 2x e 3x a sua frequencia: so a fundamental
   tem todos os harmonicos acendendo junto. */
static int escolherFaixaFundamental(const float *faixas_antes, const float *faixas_depois, int quantidade_faixas, int faixa_minima){
    int melhor_faixa = 0;
    float melhor_pontuacao = -1.0f;

    for(int faixa = 0; faixa < quantidade_faixas; faixa++){
        int indice_fft = faixa_minima + faixa;
        float pontuacao = aumentoPerto(faixas_antes, faixas_depois, quantidade_faixas, faixa)
                        + PESO_SEGUNDO_HARMONICO * aumentoPerto(faixas_antes, faixas_depois, quantidade_faixas, 2 * indice_fft - faixa_minima)
                        + PESO_TERCEIRO_HARMONICO * aumentoPerto(faixas_antes, faixas_depois, quantidade_faixas, 3 * indice_fft - faixa_minima);

        if(pontuacao > melhor_pontuacao){
            melhor_pontuacao = pontuacao;
            melhor_faixa = faixa;
        }
    }

    return melhor_faixa;
}

/* Frequencia em Hz -> altura em semitons (69 = La de 440 Hz, 12 semitons por oitava). */
static float converterParaSemitons(float frequencia){
    return NUMERO_NOTA_LA_440 + SEMITONS_POR_OITAVA * log2f(frequencia / FREQUENCIA_LA_440);
}

int detectarNotasMelodia(const float *amostras, int quantidade_amostras, int taxa_amostragem, NotaMelodia **notas){
    int tamanho = TAMANHO_FFT_MELODIA;
    *notas = NULL;

    if(quantidade_amostras < tamanho){
        return 0;
    }

    int quantidade_analises = (quantidade_amostras - tamanho) / PASSO_FFT_MELODIA + 1;
    float segundos_por_analise = (float)PASSO_FFT_MELODIA / taxa_amostragem;

    // Faixas da FFT dentro da melodia (a faixa k corresponde a k * taxa / tamanho Hz)
    int faixa_minima = (int)ceilf(FREQUENCIA_MINIMA_MELODIA * tamanho / taxa_amostragem);
    int faixa_maxima = (int)(FREQUENCIA_MAXIMA_MELODIA * tamanho / taxa_amostragem);
    if(faixa_maxima > tamanho / 2 - 1){
        faixa_maxima = tamanho / 2 - 1;
    }
    int quantidade_faixas = faixa_maxima - faixa_minima + 1;

    float *janela_hann = malloc(tamanho * sizeof(float));
    float *real = malloc(tamanho * sizeof(float));
    float *imaginario = malloc(tamanho * sizeof(float));
    float *espectro = malloc((size_t)quantidade_analises * quantidade_faixas * sizeof(float));
    float *fluxo = calloc(quantidade_analises, sizeof(float));
    NotaMelodia *encontradas = malloc(quantidade_analises * sizeof(NotaMelodia));

    if(janela_hann == NULL || real == NULL || imaginario == NULL || espectro == NULL || fluxo == NULL || encontradas == NULL){
        free(janela_hann);
        free(real);
        free(imaginario);
        free(espectro);
        free(fluxo);
        free(encontradas);
        return -1;
    }

    double pi = acos(-1.0);
    for(int i = 0; i < tamanho; i++){
        janela_hann[i] = (float)(0.5 - 0.5 * cos(2.0 * pi * i / (tamanho - 1)));
    }

    // Tabela de senos e cossenos da FFT, calculada uma vez para a musica inteira
    float cossenos[TAMANHO_FFT_MELODIA / 2];
    float senos[TAMANHO_FFT_MELODIA / 2];
    for(int k = 0; k < tamanho / 2; k++){
        cossenos[k] = (float)cos(-2.0 * pi * k / tamanho);
        senos[k] = (float)sin(-2.0 * pi * k / tamanho);
    }

    // Passos 1 e 2: espectro de cada pedaco, comprimido com log
    for(int analise = 0; analise < quantidade_analises; analise++){
        const float *pedaco = amostras + (size_t)analise * PASSO_FFT_MELODIA;

        for(int i = 0; i < tamanho; i++){
            real[i] = pedaco[i] * janela_hann[i];
            imaginario[i] = 0.0f;
        }

        calcularFFT(real, imaginario, tamanho, cossenos, senos);

        float *faixas_agora = espectro + (size_t)analise * quantidade_faixas;

        for(int faixa = 0; faixa < quantidade_faixas; faixa++){
            int k = faixa_minima + faixa;
            float magnitude = sqrtf(real[k] * real[k] + imaginario[k] * imaginario[k]);
            // log(1 + C*x): aproxima sons fortes e fracos, senao so o mais alto contaria
            faixas_agora[faixa] = logf(1.0f + COMPRESSAO_ESPECTRO * magnitude);
        }
    }

    // Passo 2b: fica so com a parte "nota" do espectro (tira bumbo, caixa e outras batidas)
    float *harmonico = separarHarmonico(espectro, quantidade_analises, quantidade_faixas);
    free(espectro);
    espectro = harmonico;

    if(espectro == NULL){
        free(janela_hann);
        free(real);
        free(imaginario);
        free(fluxo);
        free(encontradas);
        return -1;
    }

    // Passo 3: fluxo espectral, quanto as faixas aumentaram de um pedaco para o seguinte
    double soma_fluxo = 0.0;

    for(int analise = 1; analise < quantidade_analises; analise++){
        const float *faixas_agora = espectro + (size_t)analise * quantidade_faixas;
        const float *faixas_antes = faixas_agora - quantidade_faixas;

        for(int faixa = 0; faixa < quantidade_faixas; faixa++){
            float aumento = faixas_agora[faixa] - faixas_antes[faixa];
            if(aumento > 0.0f){
                fluxo[analise] += aumento;
            }
        }

        soma_fluxo += fluxo[analise];
    }

    int quantidade_notas = 0;
    float media_fluxo = (float)(soma_fluxo / quantidade_analises);

    if(media_fluxo > 0.0f){
        // Divide pela media da musica toda: os limites passam a valer para musica alta ou baixa
        for(int analise = 0; analise < quantidade_analises; analise++){
            fluxo[analise] /= media_fluxo;
        }

        int vizinhos_pico = (int)(SEGUNDOS_PICO_FLUXO / segundos_por_analise);
        int vizinhos_media = (int)(SEGUNDOS_MEDIA_FLUXO / segundos_por_analise);

        // Passo 4: picos do fluxo bem acima da media dos vizinhos
        for(int analise = 1; analise < quantidade_analises; analise++){
            int e_pico = 1;

            for(int distancia = 1; distancia <= vizinhos_pico && e_pico; distancia++){
                if(analise - distancia >= 0 && fluxo[analise - distancia] > fluxo[analise]){
                    e_pico = 0;
                }
                if(analise + distancia < quantidade_analises && fluxo[analise + distancia] >= fluxo[analise]){
                    e_pico = 0;
                }
            }

            if(!e_pico){
                continue;
            }

            float soma_vizinhos = 0.0f;
            int contados = 0;
            for(int vizinho = analise - vizinhos_media; vizinho <= analise + vizinhos_media; vizinho++){
                if(vizinho >= 0 && vizinho < quantidade_analises){
                    soma_vizinhos += fluxo[vizinho];
                    contados++;
                }
            }
            float media_local = soma_vizinhos / contados;

            if(fluxo[analise] <= media_local * FATOR_LIMIAR_MELODIA + LIMIAR_MINIMO_MELODIA){
                continue;
            }

            // Passo 5: a altura da nota, medida logo depois do ataque
            int depois = analise + ANALISES_ATE_ALTURA;
            if(depois >= quantidade_analises){
                depois = quantidade_analises - 1;
            }

            const float *faixas_antes = espectro + (size_t)(analise - 1) * quantidade_faixas;
            const float *faixas_depois = espectro + (size_t)depois * quantidade_faixas;
            int faixa_da_nota = escolherFaixaFundamental(faixas_antes, faixas_depois, quantidade_faixas, faixa_minima);

            // A nota continua soando enquanto a faixa dela fica acima de FRACAO_SUSTENTACAO do inicio
            float forca_inicial = faixas_depois[faixa_da_nota];
            int fim = depois + 1;
            while(fim < quantidade_analises &&
                  espectro[(size_t)fim * quantidade_faixas + faixa_da_nota] >= forca_inicial * FRACAO_SUSTENTACAO){
                fim++;
            }

            float frequencia = (float)(faixa_minima + faixa_da_nota) * taxa_amostragem / tamanho;

            // O pico do fluxo aparece quando o ataque ainda esta chegando no meio do pedaco (onde a
            // janela de Hann pesa mais): medindo com notas de tempo conhecido, o ataque real fica
            // ANALISES_ATE_ATAQUE passos depois do meio do pedaco
            encontradas[quantidade_notas].tempo = ((float)(analise + ANALISES_ATE_ATAQUE) * PASSO_FFT_MELODIA + tamanho / 2) / taxa_amostragem;
            encontradas[quantidade_notas].altura = converterParaSemitons(frequencia);
            encontradas[quantidade_notas].forca = fluxo[analise] - media_local;
            encontradas[quantidade_notas].sustentacao = (fim - analise) * segundos_por_analise;
            quantidade_notas++;
        }
    }

    free(janela_hann);
    free(real);
    free(imaginario);
    free(espectro);
    free(fluxo);

    *notas = encontradas;
    return quantidade_notas;
}
