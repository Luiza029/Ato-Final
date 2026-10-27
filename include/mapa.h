#ifndef MAPA_H
#define MAPA_H
#include "listas_notas.h"

/* Lê o arquivo do mapa e insere as notas na lista. */
int carregarMapa(Listas_notas *lista, const char *caminho_mapa);
#endif