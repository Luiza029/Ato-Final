#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "listas_notas.h"
#include "constantes.h"


/* Deixa a lista vazia (ponteiros em NULL, quantidade 0). */
void criarLista(Listas_notas *lista){
    lista->inicio = NULL;
    lista->fim = NULL;
    lista->cursor = NULL;
    lista->quantidade_de_notas = 0;
}


/* Cria uma nota PENDENTE e a coloca no fim da lista em O(1). */
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


/* Acha, a partir do cursor, a nota PENDENTE da lane dentro de JANELA_BOM do tempo.
   Exige a lista ordenada; retorna NULL se não achar. */
Nota *buscarNota(Listas_notas *lista, int lane, float tempo){

    Nota *atual = lista->cursor;

    while (atual != NULL){
        if(atual->tempo > tempo + JANELA_BOM){
            return NULL;
        }

        if (atual->lane == lane && atual->estado == PENDENTE && fabsf(atual->tempo - tempo) <= JANELA_BOM){
            return atual;
        }
        atual = atual->proximo;
    }

    return NULL;
}


/* Devolve a nota do cursor e avança o cursor; NULL se as notas acabaram. */
Nota *processarProximaNota(Listas_notas *lista){

    Nota *atual = lista->cursor;

    if(atual == NULL){
        return NULL;
    }
    lista->cursor = atual->proximo;

    return atual;
}


/* Grava o resultado (PERFEITO, BOM ou ERRADO) na nota. */
void atualizarEstadoNota(Nota *nota, EstadoNota estado){
    if(nota == NULL){
        return;
    }

    nota->estado = estado;
}


/* Rewind: recua o cursor N notas e devolve cada uma a PENDENTE. */
void voltarCursor(Listas_notas *lista, int notas_para_voltar){
    if(lista->fim == NULL){
        return;
    }

    while(notas_para_voltar > 0){
        if(lista->cursor == NULL){
            lista->cursor = lista->fim;

        }else if(lista->cursor->anterior != NULL){
            lista->cursor = lista->cursor->anterior;

        }else{
            break;
        }
        lista->cursor->estado = PENDENTE;

        notas_para_voltar--;
    }
}


/* Libera todas as notas e deixa a lista vazia. */
void liberarLista(Listas_notas *lista){
    Nota *atual = lista->inicio;

    while(atual != NULL){
        Nota *proxima = atual->proximo;
        free(atual);

        atual = proxima;
    }

    lista->inicio = NULL;
    lista->fim = NULL;
    lista->cursor = NULL;
    lista->quantidade_de_notas = 0;
}


/* Remove uma nota conhecida em O(1), religando os vizinhos. */
void removerNota(Listas_notas *lista, Nota *nota){
    if(lista == NULL || nota == NULL){
        return;
    }

    if(lista->cursor == nota){
        lista->cursor = nota->proximo;
    }

    if(nota->anterior != NULL){
        nota->anterior->proximo = nota->proximo;
    
    }else{
        lista->inicio = nota->proximo;
    }

    if(nota->proximo != NULL){
        nota->proximo->anterior = nota->anterior;
    
    }else{
        lista->fim = nota->anterior;
    }

    free(nota);
    lista->quantidade_de_notas--;
}

/* Converte o estado em texto, só para impressão. */
static const char *nomeEstado(EstadoNota estado){
    switch (estado){
        case PENDENTE: return "PENDENTE";
        case PERFEITO: return "PERFEITO";
        case BOM:      return "BOM";
        case ERRADO:   return "ERRADO";
    }
    return "DESCONHECIDO";
}


/* Imprime as notas nos dois sentidos; '->' marca o cursor. */
void imprimirLista(Listas_notas *lista){
    Nota *atual = lista->inicio;
    int posicao = 0;

    printf("Lista com %d nota(s):\n", lista->quantidade_de_notas);

    printf("Frente para Tras: \n");
    while (atual != NULL){
        printf("%s[%d] tempo=%.2f lane=%d estado=%s injetada=%d\n", (atual == lista->cursor) ? "-> " : "   ", posicao, atual->tempo, atual->lane, nomeEstado(atual->estado), atual->injetada);

        atual = atual->proximo;
        posicao++;
    }

    printf("Tras para frente: \n");
    atual = lista->fim;
    posicao = lista->quantidade_de_notas - 1;
    while (atual != NULL){
        printf("%s[%d] tempo=%.2f lane=%d estado=%s injetada=%d\n", (atual == lista->cursor) ? "-> " : "   ", posicao, atual->tempo, atual->lane, nomeEstado(atual->estado), atual->injetada);

        atual = atual->anterior;
        posicao--;
    }


    if (lista->cursor == NULL){
        printf("   (cursor no fim)\n");
    }
}


/* Conta quantas notas estão no estado informado. */
int contarNotasPorEstado(Listas_notas *lista, EstadoNota estado){
    Nota *atual = lista->inicio;
    int contador = 0;
    
    while(atual != NULL){
        if(atual->estado == estado){
            contador++;
        }
        atual = atual->proximo;
    }

    return contador;
}


/* Fração de acertos (PERFEITO ou BOM), de 0 a 1, como PERCENTUAL_MINIMO_VITORIA.
   Lista vazia retorna 0. */
float calcularPercentualAcertos(Listas_notas *lista){
    if(lista->quantidade_de_notas == 0){
        return 0.0f;
    }

    int acertos = contarNotasPorEstado(lista, PERFEITO) + contarNotasPorEstado(lista, BOM);

    return (float)acertos / (float)lista->quantidade_de_notas;
}


/* Devolve a nota do cursor sem avançar. */
Nota *espiarProximaNota(Listas_notas *lista){

    if(lista == NULL){
        return NULL;
    }

    return lista->cursor;
}
