#include "raylib.h"
#include "sons.h"
#include "constantes.h"

/*
 * Efeitos sonoros curtos. Diferente da musica (Music, lida aos poucos do arquivo),
 * cada efeito e um Sound: carregado inteiro na memoria uma vez so, para tocar na hora
 * em que a tecla e apertada. O volume geral do jogo (SetMasterVolume) vale para eles tambem.
 */

static Sound som_confirmar;
static Sound som_voltar;
static Sound som_rewind;
static bool tem_som_confirmar = false;
static bool tem_som_voltar = false;
static bool tem_som_rewind = false;

/* Carrega um efeito, se o arquivo existir, e ajusta o volume dele. Retorna se deu certo. */
static bool carregarEfeito(Sound *som, const char *caminho, float volume){
    if(!FileExists(caminho)){
        return false;
    }

    *som = LoadSound(caminho);

    if(!IsSoundValid(*som)){
        return false;
    }

    SetSoundVolume(*som, volume);
    return true;
}

void carregarSons(void){
    tem_som_confirmar = carregarEfeito(&som_confirmar, CAMINHO_SOM_CONFIRMAR, VOLUME_SOM_CONFIRMAR);
    tem_som_voltar = carregarEfeito(&som_voltar, CAMINHO_SOM_VOLTAR, VOLUME_SOM_VOLTAR);
    tem_som_rewind = carregarEfeito(&som_rewind, CAMINHO_SOM_REWIND, VOLUME_SOM_REWIND);
}

void descarregarSons(void){
    if(tem_som_confirmar){
        UnloadSound(som_confirmar);
        tem_som_confirmar = false;
    }

    if(tem_som_voltar){
        UnloadSound(som_voltar);
        tem_som_voltar = false;
    }

    if(tem_som_rewind){
        UnloadSound(som_rewind);
        tem_som_rewind = false;
    }
}

void tocarSomConfirmar(void){
    if(tem_som_confirmar){
        PlaySound(som_confirmar);
    }
}

void tocarSomVoltar(void){
    if(tem_som_voltar){
        PlaySound(som_voltar);
    }
}

void tocarSomRewind(void){
    if(tem_som_rewind){
        PlaySound(som_rewind);
    }
}
