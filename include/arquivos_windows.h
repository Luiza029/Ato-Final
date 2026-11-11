#ifndef ARQUIVOS_WINDOWS_H
#define ARQUIVOS_WINDOWS_H

/* Todos os caminhos aqui sao texto UTF-8, o mesmo formato que a raylib usa
   (por exemplo, nos arquivos arrastados para a janela). */

/* Abre a janela "Abrir arquivo" do Windows filtrando musicas (.mp3, .ogg, .wav).
   janela e o GetWindowHandle() da raylib, para a janela ficar na frente do jogo.
   Retorna 1 e escreve o caminho em destino se o jogador escolheu um arquivo, 0 se cancelou. */
int escolherArquivoMusica(void *janela, char *destino, int tamanho_destino);

/* Copia um arquivo. Funciona mesmo com acentos no caminho, o que fopen nao garante no Windows.
   Retorna 1 se copiou e 0 se o arquivo de origem nao existe ou nao foi possivel gravar. */
int copiarArquivo(const char *caminho_origem, const char *caminho_destino);

#endif
