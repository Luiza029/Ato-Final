#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "listas_notas.h"
#include "constantes.h"


void criarLista(Listas_notas *lista){
    lista->inicio = NULL;
    lista->fim = NULL;
    lista->cursor = NULL;
    lista->quantidade_de_notas = 0;
}


void inserirNota(Listas_notas *lista, float tempo, int lane, int injetada){

    Nota *nova = malloc(sizeof(Nota));
    if(nova == NULL){
        return;
    }

    nova->tempo = tempo;
    nova->lane = lane;
    nova->estado = PENDENTE;
    nova->injetada = injetada;
    nova->anterior = NULL;
    nova->proximo = NULL;

    if(lista->inicio == NULL){
        lista->inicio = nova;
        lista->fim = nova;
        lista->cursor = nova;

    }else{
        lista->fim->proximo = nova;
        nova->anterior = lista->fim;
        lista->fim = nova;
    }
    lista->quantidade_de_notas++;
}


Nota *buscarNota(Listas_notas *lista, int lane, float tempo){

    Nota *atual = lista->inicio;

    while (atual != NULL){
        if (atual->lane == lane && atual->estado == PENDENTE && fabsf(atual->tempo - tempo) <= JANELA_BOM){
            return atual;
        }
        atual = atual->proximo;
    }

    return NULL;
}


Nota *processarProximaNota(Listas_notas *lista){

    Nota *atual = lista->cursor;

    if (atual == NULL){
        return NULL;
    }
    lista->cursor = atual->proximo;

    return atual;
}


void atualizarEstadoNota(Nota *nota, EstadoNota estado){
    if (nota == NULL){
        return;
    }

    nota->estado = estado;
}


void voltarCursor(Listas_notas *lista, int notas_para_voltar){
    if (lista->fim == NULL){
        return;
    }

    while (notas_para_voltar > 0){
        if (lista->cursor == NULL){
            lista->cursor = lista->fim;

        }else if (lista->cursor->anterior != NULL){
            lista->cursor = lista->cursor->anterior;

        }else{
            break;
        }
        lista->cursor->estado = PENDENTE;

        notas_para_voltar--;
    }
}


void liberarLista(Listas_notas *lista){
    Nota *atual = lista->inicio;

    while (atual != NULL){
        Nota *proxima = atual->proximo;
        free(atual);

        atual = proxima;
    }

    lista->inicio = NULL;
    lista->fim = NULL;
    lista->cursor = NULL;
    lista->quantidade_de_notas = 0;
}


void removerNota(Listas_notas *lista, Nota *nota){
    if (lista == NULL || nota == NULL){
        return;
    }

    if (lista->cursor == nota){
        lista->cursor = nota->proximo;
    }

    if (nota->anterior != NULL){
        nota->anterior->proximo = nota->proximo;
    
    }else{
        lista->inicio = nota->proximo;
    }

    if (nota->proximo != NULL){
        nota->proximo->anterior = nota->anterior;
    
    }else{
        lista->fim = nota->anterior;
    }

    free(nota);
    lista->quantidade_de_notas--;
}

static const char *nomeEstado(EstadoNota estado){
    switch (estado){
        case PENDENTE: return "PENDENTE";
        case PERFEITO: return "PERFEITO";
        case BOM:      return "BOM";
        case ERRADO:   return "ERRADO";
    }
    return "DESCONHECIDO";
}


void imprimirLista(Listas_notas *lista){
    Nota *atual = lista->inicio;
    int posicao = 0;

    printf("Lista com %d nota(s):\n", lista->quantidade_de_notas);

    while (atual != NULL){
        printf("%s[%d] tempo=%.2f lane=%d estado=%s injetada=%d\n", (atual == lista->cursor) ? "-> " : "   ", posicao, atual->tempo, atual->lane, nomeEstado(atual->estado), atual->injetada);

        atual = atual->proximo;
        posicao++;
    }

    if (lista->cursor == NULL){
        printf("   (cursor no fim)\n");
    }
}