#ifndef GERADOR_H
#define GERADOR_H

#include "constantes.h"
#include "menu.h"

/* Monta o caminho do mapa de uma musica numa dificuldade.
   Ex.: "Downloads/fuga.ogg" no medio vira "mapas/fuga_medio.txt". */
void montarCaminhoMapa(char *destino, int tamanho_destino, const char *caminho_musica, Dificuldade dificuldade);

/* Le a musica, detecta as batidas e salva os mapas facil, medio e dificil em PASTA_MAPAS.
   Retorna 1 se deu certo e 0 se nao conseguiu ler a musica ou salvar algum mapa. */
int gerarMapas(const char *caminho_musica);

/* Deixa os tres mapas da musica prontos: usa os que ja existem ou gera agora.
   Se der certo, escreve os caminhos em caminhos_mapas e retorna 1. Se falhar, nao mexe neles e retorna 0. */
int prepararMapasDaMusica(const char *caminho_musica, char caminhos_mapas[QUANTIDADE_DIFICULDADES][TAMANHO_MAXIMO_CAMINHO]);

#endif
