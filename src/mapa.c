#include <stdio.h>
#include "mapa.h"
#include "ordenacao.h"
#include "constantes.h"

int carregarMapa(Listas_notas *lista, const char *caminho_mapa){
    FILE *arq = fopen(caminho_mapa, "r");

    if(arq == NULL){
        return 0;
    }

    char linha[100];
    int lane;
    float tempo;

    while (fgets(linha, sizeof(linha), arq) != NULL){
        if(sscanf(linha, "%f %d", &tempo, &lane) != 2){
            continue;
        }

        if(lane < 0 || lane > 3 || tempo < 0){
            continue;
        } 
        
        inserirNota(lista, tempo, lane, 0);
    }

    fclose(arq);
    ordenarNotas(lista);
    return 1;
}


/* Grava as notas do mapa (nao injetadas) no arquivo, uma por linha: tempo lane.
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

        fprintf(arq, "%.3f %d\n", nota->tempo, nota->lane);
    }

    fclose(arq);
    return 1;
}
