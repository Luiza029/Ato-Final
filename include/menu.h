#ifndef MENU_H
#define MENU_H

typedef enum OpcaoMenu{
    MENU_JOGAR,
    MENU_COMO_JOGAR,
    MENU_SAIR,
    QUANTIDADE_OPCOES_MENU
} OpcaoMenu;

void carregarMenu(void);
void descarregarMenu(void);
void desenharMenu(OpcaoMenu opcao_selecionada);
int atualizarMenu(OpcaoMenu *opcao_selecionada);
void desenharComoJogar(void);

#endif