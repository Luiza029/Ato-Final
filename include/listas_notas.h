#ifndef listas_notas_h
#define listas_notas_h

typedef enum EstadoNota{PENDENTE, PERFEITO, BOM, ERRADO} EstadoNota;

typedef struct Nota{
    float tempo;
    int lane;
    EstadoNota estado;
    int injetada;
    struct Nota *anterior;
    struct Nota *proximo;

}Nota;

typedef struct Listas_notas{
    Nota* inicio;
    Nota* fim;
    Nota *cursor;
    int quantidade_de_notas;
}Listas_notas;

void criarLista(Listas_notas *lista);
void inserirNota(Listas_notas *lista, float tempo, int lane, int injetada);
void removerNota(Listas_notas *lista, Nota *nota);
Nota *buscarNota(Listas_notas *lista, int lane, float tempo);
Nota *processarProximaNota(Listas_notas *lista);
void voltarCursor(Listas_notas *lista, int notas_para_voltar);
void atualizarEstadoNota(Nota *nota, EstadoNota estado);
void imprimirLista(Listas_notas *lista);
void liberarLista(Listas_notas *lista);
void ordenarNotas(Listas_notas *lista);
void imprimirListaInversa(Listas_notas *lista);
int contarNotasPorEstado(Listas_notas *lista, EstadoNota estado);
float calcularPercentualAcertos(Listas_notas *lista);
Nota *espiarProximaNota(Listas_notas *lista);

#endif