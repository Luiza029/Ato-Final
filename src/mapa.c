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