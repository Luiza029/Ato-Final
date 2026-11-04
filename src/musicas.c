#include <stdio.h>
#include <ctype.h>
#include "raylib.h"
#include "musicas.h"
#include "gerador.h"
#include "constantes.h"
#include "arquivos_windows.h"

/*
 * Biblioteca de musicas do jogo: todas ficam em PASTA_MUSICAS.
 * Ao adicionar uma musica nova, o arquivo e copiado para la (assim ela continua
 * disponivel nas proximas vezes que o jogo abrir) e os mapas sao gerados na hora.
 * A copia ganha um nome sem acentos e espacos, para o resto do jogo abrir sem problema.
 */

/* Letras acentuadas em UTF-8 sao dois bytes: 0xC3 e um segundo byte de 0x80 a 0xBF.
   Os 5 bits finais do segundo byte dizem qual e a letra (iguais para maiuscula e minuscula):
   0 a 6 = A com acento, 7 = C cedilha, 8 a 11 = E, ... ('_' = simbolo que nao vira letra). */
static const char LETRAS_SEM_ACENTO[] = "aaaaaaaceeeeiiiidnooooo_ouuuuy_y";

/* Deixa o nome minusculo, tira os acentos (a cedilha vira c, o til some) e troca o resto por '_'.
   Evita acentos e espacos, que dao problema para abrir arquivos no Windows. */
static void limparNomeArquivo(char *destino, int tamanho_destino, const char *nome_original){
    int tamanho = 0;

    for(int i = 0; nome_original[i] != '\0' && tamanho < tamanho_destino - 1; i++){
        unsigned char letra = (unsigned char)nome_original[i];
        unsigned char proximo = (unsigned char)nome_original[i + 1];

        if(letra == 0xC3 && proximo >= 0x80 && proximo <= 0xBF){
            letra = (unsigned char)LETRAS_SEM_ACENTO[proximo & 0x1F];
            i++;
        }

        if(letra < 128 && isalnum(letra)){
            destino[tamanho++] = (char)tolower(letra);
        }

        // Um '_' no lugar de cada trecho de espacos, acentos ou simbolos (nunca dois seguidos nem no inicio)
        else if(tamanho > 0 && destino[tamanho - 1] != '_'){
            destino[tamanho++] = '_';
        }
    }

    while(tamanho > 0 && destino[tamanho - 1] == '_'){
        tamanho--;
    }

    destino[tamanho] = '\0';

    if(tamanho == 0){
        snprintf(destino, tamanho_destino, "musica");
    }
}

FilePathList carregarListaMusicas(void){
    if(!DirectoryExists(PASTA_MUSICAS)){
        MakeDirectory(PASTA_MUSICAS);
    }

    return LoadDirectoryFilesEx(PASTA_MUSICAS, EXTENSOES_MUSICA, false);
}

void descarregarListaMusicas(FilePathList musicas){
    UnloadDirectoryFiles(musicas);
}

int adicionarMusica(const char *caminho_origem, char *caminho_copia, int tamanho_copia){
    // So olha a extensao: com acento no caminho, FileExists falha no Windows mesmo com o arquivo existindo
    if(!IsFileExtension(caminho_origem, EXTENSOES_MUSICA)){
        return 0;
    }

    char nome[TAMANHO_MAXIMO_NOME_MUSICA];
    limparNomeArquivo(nome, TAMANHO_MAXIMO_NOME_MUSICA, GetFileNameWithoutExt(caminho_origem));
    snprintf(caminho_copia, tamanho_copia, "%s/%s%s", PASTA_MUSICAS, nome, TextToLower(GetFileExtension(caminho_origem)));

    // Se uma musica com esse nome ja esta na pasta, so reaproveita
    int copiou_agora = 0;

    if(!FileExists(caminho_copia)){
        if(!DirectoryExists(PASTA_MUSICAS)){
            MakeDirectory(PASTA_MUSICAS);
        }

        // A copia tem nome sem acento, entao depois disso tudo funciona com as funcoes normais
        if(!copiarArquivo(caminho_origem, caminho_copia)){
            return 0;
        }

        copiou_agora = 1;
    }

    char caminhos_mapas[QUANTIDADE_DIFICULDADES][TAMANHO_MAXIMO_CAMINHO];

    if(!prepararMapasDaMusica(caminho_copia, caminhos_mapas)){
        // Arquivo com extensao certa mas conteudo que nao abre: nao deixa a copia na lista
        if(copiou_agora){
            remove(caminho_copia);
        }

        return 0;
    }

    return 1;
}

void montarNomeExibicao(char *destino, int tamanho_destino, const char *caminho_musica){
    snprintf(destino, tamanho_destino, "%s", GetFileNameWithoutExt(caminho_musica));

    for(int i = 0; destino[i] != '\0'; i++){
        if(destino[i] == '_'){
            destino[i] = ' ';
        }
    }
}
