# Usa o gcc do MSYS2 se existir; senão, o gcc que estiver no PATH
ifneq ($(wildcard C:/msys64/ucrt64/bin/gcc.exe),)
    CC = C:/msys64/ucrt64/bin/gcc.exe
else
    CC = gcc
endif

CFLAGS = -Wall -Wextra -std=c99 -Iinclude -Ivendor/raylib/include
LIBS = -Lvendor/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm

SRC = $(wildcard src/*.c)
HEADERS = $(wildcard include/*.h)
EXEC = jogo.exe

# Compila o jogo completo (regra padrão)
$(EXEC): $(SRC) $(HEADERS)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LIBS)

# Compila e executa o jogo
rodar: $(EXEC)
	"$(CURDIR)/$(EXEC)"

# Teste da lista no terminal, sem raylib
teste: testes/teste_lista.c src/listas_notas.c src/ordenacao.c
	$(CC) -Wall -Wextra -std=c99 -Iinclude $^ -o teste_lista.exe
	"$(CURDIR)/teste_lista.exe"

# Apaga os executáveis
limpar:
	del /Q *.exe

.PHONY: rodar teste limpar