#ifndef MELODIA_H
#define MELODIA_H

/* Uma nota encontrada na faixa de frequencia da melodia. */
typedef struct NotaMelodia{
    float tempo;        // inicio da nota, em segundos
    float altura;       // altura em semitons (69 = La de 440 Hz): quanto maior, mais aguda
    float forca;        // tamanho do "salto" no espectro: notas mais marcadas tem forca maior
    float sustentacao;  // por quanto tempo a nota continua soando depois de comecar, em segundos
} NotaMelodia;

/* Procura o comeco das notas da melodia na musica (fluxo espectral, explicado em melodia.c).
   amostras: o som em mono, com valores de -1 a 1.
   Escreve em *notas um vetor alocado com malloc, em ordem de tempo (liberar com free).
   Retorna quantas notas achou, ou -1 se faltou memoria. */
int detectarNotasMelodia(const float *amostras, int quantidade_amostras, int taxa_amostragem, NotaMelodia **notas);

#endif
