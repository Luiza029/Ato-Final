#include <stdio.h>
#include "mapa.h"
#include "ordenacao.h"
#include "constantes.h"

/* Le o arquivo do mapa (uma nota por linha: tempo lane, e a duracao opcional)
   e insere as notas na lista, ja ordenada. Retorna 0 se o arquivo nao abriu. */
int carregarMapa(Listas_notas *lista, const char *caminho_mapa){
    FILE *arq = fopen(caminho_mapa, "r");

    if(arq == NULL){
        return 0;
    }

    char linha[100];
    int lane;
    float tempo;
    float duracao;

    while (fgets(linha, sizeof(linha), arq) != NULL){
        // A 3a coluna (duracao da nota longa) e opcional: mapas antigos tem so 2
        int lidos = sscanf(linha, "%f %d %f", &tempo, &lane, &duracao);
        if(lidos < 2){
            continue;
        }

        if(lane < 0 || lane > 3 || tempo < 0){
            continue;
        } 
        
        inserirNota(lista, tempo, lane, 0);

        if(lidos == 3 && duracao > 0.0f){
            lista->fim->duracao = duracao;
        }
    }

    fclose(arq);
    ordenarNotas(lista);
    return 1;
}


/* Grava as notas do mapa (nao injetadas) no arquivo, uma por linha: tempo lane
   (e a duracao, so nas notas longas).
   Retorna 1 se deu certo e 0 se nao conseguiu criar o arquivo. */
int salvarMapa(const Listas_notas *lista, const char *caminho_mapa){
    FILE *arq = fopen(caminho_mapa, "w");

    if(arq == NULL){
        return 0;
    }

    for(const Nota *nota = lista->inicio; nota != NULL; nota = nota->proximo){
        if(nota->injetada){
            continue;
        }

        if(nota->duracao > 0.0f){
            fprintf(arq, "%.3f %d %.3f\n", nota->tempo, nota->lane, nota->duracao);
        }else{
            fprintf(arq, "%.3f %d\n", nota->tempo, nota->lane);
        }
    }

    fclose(arq);
    return 1;
}
