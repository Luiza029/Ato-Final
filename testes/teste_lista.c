#include <stdio.h>
#include "listas_notas.h"
#include "ordenacao.h"

static int falhas = 0;

/* Mostra OK ou FALHOU para cada verificação e conta as falhas. */
static void conferir(const char *descricao, int condicao){
    if(condicao){
        printf("[OK]     %s\n", descricao);
    }else{
        printf("[FALHOU] %s\n", descricao);
        falhas++;
    }
}

/* Exercita todas as funções da lista e da ordenação. */
int main(void){
    Listas_notas lista;
    criarLista(&lista);
    conferir("lista nova está vazia", lista.inicio == NULL && lista.fim == NULL && lista.cursor == NULL && lista.quantidade_de_notas == 0);
    conferir("percentual de lista vazia é 0", calcularPercentualAcertos(&lista) == 0.0f);

    inserirNota(&lista, 3.0f, 0, 0);
    inserirNota(&lista, 1.0f, 1, 0);
    inserirNota(&lista, 2.0f, 2, 0);
    inserirNota(&lista, 0.5f, 3, 0);
    conferir("4 notas inseridas", lista.quantidade_de_notas == 4);
    conferir("cursor na primeira nota inserida", lista.cursor == lista.inicio);
    conferir("fim é a última nota inserida", lista.fim->tempo == 0.5f);

    ordenarNotas(&lista);
    imprimirLista(&lista);
    conferir("ordenada: primeira nota tem tempo 0.5", lista.inicio->tempo == 0.5f);
    conferir("ordenada: última nota tem tempo 3.0", lista.fim->tempo == 3.0f);
    conferir("cursor no início depois de ordenar", lista.cursor == lista.inicio);

    int notas_frente = 0;
    for(Nota *atual = lista.inicio; atual != NULL; atual = atual->proximo){
        notas_frente++;
    }
    int notas_tras = 0;
    for(Nota *atual = lista.fim; atual != NULL; atual = atual->anterior){
        notas_tras++;
    }
    conferir("percorrer para frente e para trás dá 4 notas", notas_frente == 4 && notas_tras == 4);

    Nota *primeira = espiarProximaNota(&lista);
    conferir("espiar devolve a nota do cursor sem avançar", primeira == lista.inicio && lista.cursor == lista.inicio);

    Nota *processada = processarProximaNota(&lista);
    conferir("processar devolve a nota do cursor", processada == primeira);
    conferir("cursor avançou para a segunda nota", lista.cursor == primeira->proximo);
    atualizarEstadoNota(processada, PERFEITO);
    conferir("estado gravado como PERFEITO", processada->estado == PERFEITO);

    Nota *achada = buscarNota(&lista, 1, 1.05f);
    conferir("busca acha a nota da lane 1 perto de 1.05", achada != NULL && achada->tempo == 1.0f);
    conferir("busca não acha a lane errada", buscarNota(&lista, 0, 1.0f) == NULL);
    conferir("busca não acha fora da janela", buscarNota(&lista, 1, 1.5f) == NULL);

    atualizarEstadoNota(achada, BOM);
    conferir("conta 1 PERFEITO", contarNotasPorEstado(&lista, PERFEITO) == 1);
    conferir("conta 1 BOM", contarNotasPorEstado(&lista, BOM) == 1);
    conferir("conta 2 PENDENTE", contarNotasPorEstado(&lista, PENDENTE) == 2);
    conferir("percentual é 0.5 (2 acertos em 4 notas)", calcularPercentualAcertos(&lista) == 0.5f);

    while(processarProximaNota(&lista) != NULL){
    }
    conferir("cursor em NULL no fim da música", lista.cursor == NULL);
    conferir("espiar devolve NULL no fim da música", espiarProximaNota(&lista) == NULL);

    voltarCursor(&lista, 2);
    conferir("rewind de 2 notas: cursor na penúltima", lista.cursor == lista.fim->anterior);
    conferir("notas do rewind voltaram a PENDENTE", lista.cursor->estado == PENDENTE && lista.fim->estado == PENDENTE);

    voltarCursor(&lista, 100);
    conferir("rewind para no início da lista", lista.cursor == lista.inicio);
    conferir("depois do rewind todas estão PENDENTE", calcularPercentualAcertos(&lista) == 0.0f);

    inserirNota(&lista, 1.5f, 2, 1);
    ordenarNotas(&lista);
    conferir("nota injetada entrou entre 1.0 e 2.0", lista.inicio->proximo->proximo->tempo == 1.5f);
    conferir("5 notas na lista", lista.quantidade_de_notas == 5);

    removerNota(&lista, lista.inicio->proximo->proximo);
    conferir("remover do meio deixa 4 notas", lista.quantidade_de_notas == 4);

    removerNota(&lista, lista.inicio);
    conferir("remover o início atualiza início e cursor", lista.inicio->tempo == 1.0f && lista.cursor == lista.inicio && lista.inicio->anterior == NULL);

    removerNota(&lista, lista.fim);
    conferir("remover o fim atualiza o fim", lista.fim->tempo == 2.0f && lista.fim->proximo == NULL);

    removerNota(&lista, lista.inicio);
    removerNota(&lista, lista.inicio);
    conferir("remover todas deixa a lista vazia", lista.inicio == NULL && lista.fim == NULL && lista.cursor == NULL && lista.quantidade_de_notas == 0);

    ordenarNotas(&lista);
    inserirNota(&lista, 1.0f, 0, 0);
    ordenarNotas(&lista);
    conferir("ordenar lista vazia ou com uma nota não quebra", lista.quantidade_de_notas == 1 && lista.inicio == lista.fim);

    liberarLista(&lista);
    conferir("liberar deixa a lista vazia", lista.inicio == NULL && lista.fim == NULL && lista.cursor == NULL && lista.quantidade_de_notas == 0);

    printf("\n%d falha(s)\n", falhas);

    Listas_notas lista_injecao;
    criarLista(&lista_injecao);
    inserirNota(&lista_injecao, 1.0f, 0, 0);
    inserirNota(&lista_injecao, 2.0f, 1, 0);
    inserirNota(&lista_injecao, 5.0f, 2, 0);
    atualizarEstadoNota(processarProximaNota(&lista_injecao), BOM);
    atualizarEstadoNota(processarProximaNota(&lista_injecao), BOM);
    conferir("cursor na nota de 5.0 antes da injeção", lista_injecao.cursor != NULL && lista_injecao.cursor->tempo == 5.0f);

    inserirNota(&lista_injecao, 3.0f, 3, 1);
    ordenarNotas(&lista_injecao);
    conferir("cursor volta para a nota injetada de 3.0", lista_injecao.cursor != NULL && lista_injecao.cursor->tempo == 3.0f);

    Nota *nota_julgada;
    while((nota_julgada = processarProximaNota(&lista_injecao)) != NULL){
        atualizarEstadoNota(nota_julgada, ERRADO);
    }
    conferir("cursor em NULL depois de julgar tudo", lista_injecao.cursor == NULL);

    float percentual = calcularPercentualAcertos(&lista_injecao);
    conferir("percentual ignora notas injetadas (2 de 3)", percentual > 0.66f && percentual < 0.67f);

    inserirNota(&lista_injecao, 6.0f, 0, 1);
    inserirNota(&lista_injecao, 6.5f, 1, 1);
    ordenarNotas(&lista_injecao);
    conferir("injeção no fim: cursor vai para a primeira injetada", lista_injecao.cursor != NULL && lista_injecao.cursor->tempo == 6.0f);

    liberarLista(&lista_injecao);

    return falhas == 0 ? 0 : 1;
}