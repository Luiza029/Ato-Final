#ifndef MENU_H
#define MENU_H

typedef enum OpcaoMenu{
    MENU_JOGAR,
    MENU_COMO_JOGAR,
    MENU_SAIR,
    QUANTIDADE_OPCOES_MENU
} OpcaoMenu;

/* Desenha o menu destacando a opcao selecionada. */
void desenharMenu(OpcaoMenu opcao_selecionada);

/* Le as setas do teclado e atualiza a opcao selecionada.
   Retorna 1 quando o jogador confirma e 0 caso contrario. */
int atualizarMenu(OpcaoMenu *opcao_selecionada);

/* Tela com as instrucoes do jogo. */
void desenharComoJogar(void);

#endif