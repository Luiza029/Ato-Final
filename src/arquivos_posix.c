/*
 * Versao de macOS/Linux das funcoes de arquivos_windows.c.
 *
 * No macOS a janela "Abrir arquivo" vem do osascript (AppleScript); no Linux
 * tentamos o zenity. Ja o fopen dessas plataformas aceita UTF-8, entao a copia
 * pode usar a biblioteca padrao.
 */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include "arquivos_windows.h"

int escolherArquivoMusica(void *janela, char *destino, int tamanho_destino){
    (void)janela;

#ifdef __APPLE__
    const char *comando =
        "osascript -e 'POSIX path of (choose file with prompt \"Adicionar musica ao Ato Final\" "
        "of type {\"mp3\", \"ogg\", \"wav\"})' 2>/dev/null";
#else
    const char *comando =
        "zenity --file-selection --title='Adicionar musica ao Ato Final' "
        "--file-filter='Musicas | *.mp3 *.ogg *.wav' 2>/dev/null";
#endif

    FILE *saida = popen(comando, "r");

    if(saida == NULL){
        return 0;
    }

    char *leu = fgets(destino, tamanho_destino, saida);
    int status = pclose(saida);

    if(leu == NULL || status != 0){
        return 0;
    }

    // Tira o \n que o comando coloca no fim do caminho
    destino[strcspn(destino, "\r\n")] = '\0';
    return destino[0] != '\0';
}

int copiarArquivo(const char *caminho_origem, const char *caminho_destino){
    FILE *origem = fopen(caminho_origem, "rb");

    if(origem == NULL){
        return 0;
    }

    FILE *destino = fopen(caminho_destino, "wb");

    if(destino == NULL){
        fclose(origem);
        return 0;
    }

    char bloco[8192];
    size_t lidos;
    int ok = 1;

    while((lidos = fread(bloco, 1, sizeof(bloco), origem)) > 0){
        if(fwrite(bloco, 1, lidos, destino) != lidos){
            ok = 0;
            break;
        }
    }

    fclose(origem);

    if(fclose(destino) != 0){
        ok = 0;
    }

    return ok;
}
