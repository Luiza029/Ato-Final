#ifndef MUSICAS_H
#define MUSICAS_H

#include "raylib.h"

/* Le a pasta PASTA_MUSICAS (cria se nao existir) e devolve os caminhos das musicas dela.
   Liberar depois com descarregarListaMusicas. */
FilePathList carregarListaMusicas(void);

void descarregarListaMusicas(FilePathList musicas);

/* Copia uma musica do computador para PASTA_MUSICAS, com um nome sem espacos e acentos,
   e ja gera os mapas dela. Escreve o caminho da copia em caminho_copia.
   Retorna 1 se deu certo e 0 se o arquivo nao e uma musica que o jogo consegue ler. */
int adicionarMusica(const char *caminho_origem, char *caminho_copia, int tamanho_copia);

/* Nome para mostrar na tela: "assets/musicas/halloween_theme.mp3" vira "halloween theme". */
void montarNomeExibicao(char *destino, int tamanho_destino, const char *caminho_musica);

#endif
