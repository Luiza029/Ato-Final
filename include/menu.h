#ifndef MENU_H
#define MENU_H

typedef enum OpcaoMenu{
    MENU_JOGAR,
    MENU_COMO_JOGAR,
    MENU_SAIR,
    QUANTIDADE_OPCOES_MENU
} OpcaoMenu;

typedef enum Dificuldade{
    DIFICULDADE_FACIL,
    DIFICULDADE_MEDIO,
    DIFICULDADE_DIFICIL,
    QUANTIDADE_DIFICULDADES
} Dificuldade;

void carregarMenu(void);
void descarregarMenu(void);
void desenharMenu(OpcaoMenu opcao_selecionada);
int atualizarMenu(OpcaoMenu *opcao_selecionada);
void desenharComoJogar(void);
int atualizarSelecaoDificuldade(Dificuldade *dificuldade_selecionada);
void desenharSelecaoDificuldade(Dificuldade dificuldade_selecionada);

#endif
