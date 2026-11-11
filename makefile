CFLAGS = -Wall -Wextra -std=c99 -Iinclude

ifeq ($(OS),Windows_NT)
    # Usa o gcc do MSYS2 se existir; senão, o gcc que estiver no PATH
    ifneq ($(wildcard C:/msys64/ucrt64/bin/gcc.exe),)
        CC = C:/msys64/ucrt64/bin/gcc.exe
    else
        CC = gcc
    endif
    CFLAGS += -Ivendor/raylib/include
    LIBS = -Lvendor/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm
    EXEC = jogo.exe
    PLATAFORMA = src/arquivos_windows.c
    EXEC_TESTE = teste_lista.exe
    APAGAR = del /Q *.exe
else
    # macOS: usa a raylib do Homebrew (brew install raylib)
    CC = cc
    CFLAGS += -I$(shell brew --prefix)/include
    LIBS = -L$(shell brew --prefix)/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
    EXEC = jogo
    PLATAFORMA = src/arquivos_posix.c
    EXEC_TESTE = teste_lista
    APAGAR = rm -f jogo teste_lista
endif

# Cada sistema usa a sua versão das funções de arquivo
SRC = $(filter-out src/arquivos_windows.c src/arquivos_posix.c,$(wildcard src/*.c)) $(PLATAFORMA)
HEADERS = $(wildcard include/*.h)

# Compila o jogo completo (regra padrão)
$(EXEC): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LIBS)

# Compila e executa o jogo
rodar: $(EXEC)
	"$(CURDIR)/$(EXEC)"

# Teste da lista no terminal, sem raylib
teste: testes/teste_lista.c src/listas_notas.c src/ordenacao.c
	$(CC) -Wall -Wextra -std=c99 -Iinclude $^ -o $(EXEC_TESTE)
	"$(CURDIR)/$(EXEC_TESTE)"

# Apaga os executáveis
limpar:
	$(APAGAR)

.PHONY: rodar teste limpar
