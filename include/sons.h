#ifndef SONS_H
#define SONS_H

/* Efeitos sonoros do jogo (confirmar, voltar e rewind).
   carregarSons precisa ser chamado depois de InitAudioDevice.
   Se algum arquivo faltar, o jogo continua normalmente, so sem aquele som. */
void carregarSons(void);
void descarregarSons(void);

void tocarSomConfirmar(void);
void tocarSomVoltar(void);
void tocarSomRewind(void);

#endif
