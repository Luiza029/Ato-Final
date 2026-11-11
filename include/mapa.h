#ifndef MAPA_H
#define MAPA_H
#include "listas_notas.h"

/* Lê o arquivo do mapa e insere as notas na lista. */
int carregarMapa(Listas_notas *lista, const char *caminho_mapa);

/* Grava as notas da lista no arquivo do mapa (tempo lane e, nas notas longas, a duracao). */
int salvarMapa(const Listas_notas *lista, const char *caminho_mapa);
#endif