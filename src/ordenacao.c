#include <stddef.h>
#include "ordenacao.h"

/* Insertion Sort por tempo, religando os nós (estável) e ajustando o cursor. */
void ordenarNotas(Listas_notas *lista){
    if(lista == NULL || lista->inicio == NULL || lista->inicio->proximo == NULL){
        return;
    }

    Nota *restante = lista->inicio->proximo;

    lista->inicio->proximo = NULL;
    lista->fim = lista->inicio;

    while(restante != NULL){
        Nota *atual = restante;
        restante = restante->proximo;
        atual->anterior = NULL;
        atual->proximo = NULL;

        Nota *posicao = lista->fim;
        while(posicao != NULL && posicao->tempo > atual->tempo){
            posicao = posicao->anterior;
        }

        if(posicao == NULL){
            atual->proximo = lista->inicio;
            lista->inicio->anterior = atual;
            lista->inicio = atual;

        }else{
        
            atual->anterior = posicao;
            atual->proximo = posicao->proximo;

            if(posicao->proximo != NULL){
                posicao->proximo->anterior = atual;
            
            }else{
                lista->fim = atual;
            }

            posicao->proximo = atual;
        }
    }

    if(lista->cursor == NULL && lista->fim->estado == PENDENTE){
        lista->cursor = lista->fim;
    }
    
    while(lista->cursor != NULL && lista->cursor->anterior != NULL && lista->cursor->anterior->estado == PENDENTE){
        lista->cursor = lista->cursor->anterior;
    }
}