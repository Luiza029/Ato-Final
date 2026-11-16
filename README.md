# Ritmo de Matar

Jogo de ritmo com temática de terror, feito em C com a biblioteca [raylib](https://www.raylib.com/) para a disciplina de Algoritmos e Estruturas de Dados (tema "Ode ao Cinema").

## Sobre o jogo

Você é um sobrevivente do incêndio da Casa do Pânico e precisa escapar do assassino que o persegue pelas ruas da cidade. A fuga acontece no ritmo da música: as notas chegam pela pauta e você precisa apertar a tecla certa na hora certa.

- **Acertou:** o assassino fica para trás.
- **Errou:** ele chega mais perto.
- **Errou demais:** o assassino sabota a pauta e joga notas extras no seu caminho.
- **Rewind:** volta a fita algumas notas para tentar de novo (uso limitado).

**Vitória:** chegar ao fim da música acertando pelo menos 75% das notas. **Derrota:** ser alcançado pelo assassino ou terminar a música abaixo de 75%.

## Requisitos

- **Windows:** `gcc` e `make` do MSYS2. A raylib já vem no projeto, na pasta `vendor/`.
- **macOS:** `cc` (Xcode Command Line Tools), `make` e a raylib do Homebrew (`brew install raylib`).

### Instalando o gcc e o make no Windows (só uma vez)

No PowerShell:

```
winget install --id MSYS2.MSYS2 -e
C:\msys64\usr\bin\bash.exe -lc "pacman -S --noconfirm mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make"
```

## Como compilar e executar

Na pasta do projeto (a que tem o `makefile`):

| Comando (Windows) | Comando (macOS) | O que faz |
|---|---|---|
| `mingw32-make` | compila o jogo |
| `mingw32-make rodar` | compila e abre o jogo |
| `mingw32-make teste` | compila e roda o teste da lista no terminal |
| `mingw32-make limpar` | apaga os executáveis |

O jogo precisa ser executado a partir da pasta do projeto, porque procura as músicas e os mapas em `assets/` e `mapas/`.

## Controles

| Tecla | Ação |
|---|---|
| `D` `F` `J` `K` | as 4 lanes (dá para trocar no menu **Teclas**) |
| segurar a tecla | notas longas (com rastro): segure até o fim do rastro |
| `Espaço` | rewind |
| `↑` `↓` | navegar nos menus |
| `←` `→` | ajustar o volume |
| `Enter` | confirmar |
| `Backspace` | voltar |

## Adicionando uma música

No menu **Jogar**, escolha **Adicionar música** ou **arraste** um arquivo `.mp3`, `.ogg` ou `.wav` para a janela do jogo. A música é copiada para `assets/musicas/` e o jogo gera sozinho os mapas fácil, médio e difícil em `mapas/`.

## Estrutura de dados e ordenação

- **Lista duplamente encadeada** (`src/listas_notas.c`): guarda as notas da música em ordem de tempo. Um **cursor** aponta para a próxima nota a ser julgada. O julgamento das teclas, o desenho da pauta e o **rewind** (que anda pelo ponteiro `anterior`) partem dele.
- **Insertion Sort** (`src/ordenacao.c`): reordena a lista religando os ponteiros quando o assassino **injeta notas** e quando um mapa é carregado ou gerado. Como a lista está quase sempre ordenada, ele roda perto de O(n).

## Organização das pastas

```
src/       código-fonte (.c)
include/   cabeçalhos (.h)
testes/    teste da lista no terminal (sem raylib)
mapas/     mapas de notas (tempo, lane e duração por linha)
assets/    músicas, sprites e imagens
vendor/    raylib pré-compilada para Windows
```

## Créditos

- Músicas: "Halloween Theme" (John Carpenter) e "This Is Halloween" (Danny Elfman), usadas apenas para fins educacionais.
- Sprite do assassino: Remi Cunha - Design 4°B
- Biblioteca gráfica: [raylib](https://www.raylib.com/) (licença zlib).

## Equipe

- Carlos Murilo de Oliveira Costa Filho
- José Milton Batista Neto
- Luiza Costa Wanderley
- Marcus Vinicius Pereira Barbosa
