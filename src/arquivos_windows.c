/*
 * Funcoes que precisam da API do Windows: a janela "Abrir arquivo" do botao
 * "Adicionar musica" e a copia de arquivos com acento no nome.
 *
 * Ficam num arquivo separado porque windows.h e raylib.h nao podem ser incluidos
 * juntos (os dois definem nomes como Rectangle, CloseWindow e DrawText).
 *
 * A raylib trabalha com caminhos em UTF-8, mas o fopen do Windows nao entende
 * acentos em UTF-8. Por isso convertemos para texto "largo" (wchar_t), que e o
 * formato nativo do Windows, e usamos as versoes W das funcoes.
 *
 * A funcao GetOpenFileNameW fica na comdlg32.dll; ela e carregada em tempo de
 * execucao com LoadLibraryA, assim o makefile nao precisa de mais uma biblioteca.
 */
#include <windows.h>
#include <commdlg.h>
#include "arquivos_windows.h"

#define TAMANHO_CAMINHO_WINDOWS 1024

typedef BOOL (WINAPI *FuncaoAbrirArquivo)(LPOPENFILENAMEW dialogo);

/* UTF-8 -> texto largo do Windows. Retorna 1 se converteu. */
static int converterParaWindows(const char *texto_utf8, wchar_t *destino, int tamanho_destino){
    return MultiByteToWideChar(CP_UTF8, 0, texto_utf8, -1, destino, tamanho_destino) > 0;
}

/* Texto largo do Windows -> UTF-8. Retorna 1 se converteu. */
static int converterParaUtf8(const wchar_t *texto_windows, char *destino, int tamanho_destino){
    return WideCharToMultiByte(CP_UTF8, 0, texto_windows, -1, destino, tamanho_destino, NULL, NULL) > 0;
}

int escolherArquivoMusica(void *janela, char *destino, int tamanho_destino){
    HMODULE comdlg32 = LoadLibraryA("comdlg32.dll");

    if(comdlg32 == NULL){
        return 0;
    }

    // O cast passa por void (*)(void) para o gcc aceitar converter o ponteiro de funcao
    FuncaoAbrirArquivo abrirArquivo = (FuncaoAbrirArquivo)(void (*)(void))GetProcAddress(comdlg32, "GetOpenFileNameW");

    if(abrirArquivo == NULL){
        FreeLibrary(comdlg32);
        return 0;
    }

    wchar_t caminho_escolhido[TAMANHO_CAMINHO_WINDOWS] = L"";
    OPENFILENAMEW dialogo;
    ZeroMemory(&dialogo, sizeof(dialogo));

    dialogo.lStructSize = sizeof(dialogo);
    dialogo.hwndOwner = (HWND)janela;
    dialogo.lpstrFilter = L"Musicas (*.mp3, *.ogg, *.wav)\0*.mp3;*.ogg;*.wav\0";
    dialogo.lpstrFile = caminho_escolhido;
    dialogo.nMaxFile = TAMANHO_CAMINHO_WINDOWS;
    dialogo.lpstrTitle = L"Adicionar musica ao Ato Final";
    // OFN_NOCHANGEDIR: sem isso o Windows muda a pasta atual e o jogo perde os caminhos relativos (mapas/, assets/)
    dialogo.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    int escolheu = abrirArquivo(&dialogo) && converterParaUtf8(caminho_escolhido, destino, tamanho_destino);

    FreeLibrary(comdlg32);
    return escolheu;
}

int copiarArquivo(const char *caminho_origem, const char *caminho_destino){
    wchar_t origem[TAMANHO_CAMINHO_WINDOWS];
    wchar_t destino[TAMANHO_CAMINHO_WINDOWS];

    if(!converterParaWindows(caminho_origem, origem, TAMANHO_CAMINHO_WINDOWS) ||
       !converterParaWindows(caminho_destino, destino, TAMANHO_CAMINHO_WINDOWS)){
        return 0;
    }

    // FALSE: pode sobrescrever o destino se ele ja existir
    return CopyFileW(origem, destino, FALSE) ? 1 : 0;
}
