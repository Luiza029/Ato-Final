#ifndef CONSTANTES_H
#define CONSTANTES_H

//                     JANELA E LAYOUT 
#define TITULO_JANELA "Ritmo de Matar"
#define LARGURA_JANELA 1280
#define ALTURA_JANELA 720
#define FPS_ALVO 60

#define ALTURA_CENA 360
#define ALTURA_BARRA_INFO 60
#define ALTURA_PAUTA (ALTURA_JANELA - ALTURA_CENA - ALTURA_BARRA_INFO)

#define POSICAO_Y_BARRA_INFO (ALTURA_CENA)
#define POSICAO_Y_PAUTA (ALTURA_CENA + ALTURA_BARRA_INFO)

#define QUANTIDADE_LANES 4
#define ALTURA_LANE (ALTURA_PAUTA / QUANTIDADE_LANES)

#define POSICAO_X_LINHA_ACERTO 160
#define TAMANHO_NOTA 40

//                     CALCULO DAS NOTAS
#define JANELA_PERFEITO 0.05f
#define JANELA_BOM 0.12f

//                         NOTAS
#define VELOCIDADE_NOTAS 500.0f
#define TEMPO_VISIBILIDADE_NOTA ((float)(LARGURA_JANELA - POSICAO_X_LINHA_ACERTO) / VELOCIDADE_NOTAS)

//                        PONTUAÇÃO
#define PONTOS_PERFEITO 300
#define PONTOS_BOM 100
#define COMBO_POR_MULTIPLICADOR 10
#define MULTIPLICADOR_MAXIMO 4

//                        ASSASSINO
#define DISTANCIA_MINIMA_ASSASSINO 0
#define DISTANCIA_MAXIMA_ASSASSINO 4
#define DISTANCIA_INICIAL_ASSASSINO (DISTANCIA_MAXIMA_ASSASSINO)
#define DISTANCIA_PERDIDA_POR_ERRO 1
#define ACERTOS_PARA_RECUPERAR 3
#define DISTANCIA_GANHA_POR_RECUPERACAO 1

//                        INJEÇÃO
#define ERROS_PARA_INJECAO 3
#define NOTAS_MINIMAS_INJECAO 1
#define NOTAS_MAXIMAS_INJECAO 2
#define ANTECEDENCIA_INJECAO 1.5f
#define TOLERANCIA_MESMO_TEMPO 0.001f

//                        REWIND
#define REWINDS_MAXIMOS 1
#define NOTAS_POR_REWIND 8
#define DURACAO_ANIMACAO_REWIND 0.6f
#define RESPIRO_APOS_REWIND 1.0f

//                        VITÓRIA
#define PERCENTUAL_MINIMO_VITORIA 0.75f

//                     TEMPO DE APOIO
#define SILENCIO_INICIAL 3.0f
#define DURACAO_FEEDBACK 0.4f
#define ESPERA_FIM_DA_MUSICA 2.0f

//                  SINCRONIA COM A MUSICA
#define AJUSTE_SINCRONIA 0.06f          
#define LIMITE_DESSINCRONIA 0.15f       
#define FATOR_CORRECAO_RELOGIO 0.1f     

//                       ARQUIVOS
#define CAMINHO_MAPA_PADRAO "mapas/mapa_teste_longo.txt"

// Mapa de cada dificuldade (por enquanto, todas usam o mapa de teste)
#define CAMINHO_MAPA_FACIL   CAMINHO_MAPA_PADRAO
#define CAMINHO_MAPA_MEDIO   CAMINHO_MAPA_PADRAO
#define CAMINHO_MAPA_DIFICIL CAMINHO_MAPA_PADRAO
#define CAMINHO_FUNDO_MENU "assets/menu_fundo.png"
#define CAMINHO_MUSICA_PADRAO "assets/musicas/halloween_theme.mp3"

//                  BIBLIOTECA DE MUSICAS
#define PASTA_MUSICAS "assets/musicas"
#define EXTENSOES_MUSICA ".mp3;.ogg;.wav"
#define TAMANHO_MAXIMO_NOME_MUSICA 64
#define TAMANHO_MENSAGEM_MUSICA 160
#define MAXIMO_MUSICAS_VISIVEIS 6
#define POSICAO_Y_LISTA_MUSICAS 230

//                         VOLUME
#define CAMINHO_CONFIGURACAO "configuracao.txt"
#define VOLUME_PADRAO 50                    // em %, as musicas baixadas costumam vir altas
#define VOLUME_MAXIMO 100
#define PASSO_VOLUME 5
#define INICIO_PREVIA_VOLUME 20.0f          // segundo da musica tocado como previa na tela de volume
#define LARGURA_BARRA_VOLUME 600
#define ALTURA_BARRA_VOLUME 32
#define POSICAO_Y_BARRA_VOLUME 320

//                   GERADOR DE MAPAS
#define PASTA_MAPAS "mapas/"
#define TAMANHO_MAXIMO_CAMINHO 512
#define AMOSTRAS_POR_JANELA_GERADOR 1024    // ~23 ms de som em 44100 Hz
#define JANELAS_HISTORICO_GERADOR 43        // ~1 s de historico para a media de energia
#define ENERGIA_MINIMA_BATIDA 0.0005f       // abaixo disso e silencio, nunca vira nota
#define SENSIBILIDADE_FACIL 1.6f            // quantas vezes acima da media para virar nota
#define SENSIBILIDADE_MEDIO 1.4f
#define SENSIBILIDADE_DIFICIL 1.25f
#define INTERVALO_MINIMO_FACIL 0.7f         // segundos entre duas notas
#define INTERVALO_MINIMO_MEDIO 0.28f
#define INTERVALO_MINIMO_DIFICIL 0.16f
#define FATOR_ACORDE_DIFICIL 3.0f           // batida muito forte no dificil vira acorde (nunca dois seguidos)

//                        INTERFACE
#define MARGEM_TELA 20
#define ESPESSURA_LINHA_ACERTO 4
#define ESPESSURA_BORDA_NOTA 5
#define OPACIDADE_NOTA_ERRADA 0.3f

//                  RESPINGO DE SANGUE (acerto)
#define DURACAO_EFEITO_SANGUE 0.45f     
#define GOTAS_SANGUE_PERFEITO 14
#define GOTAS_SANGUE_BOM 7
#define VELOCIDADE_MINIMA_GOTA 90.0f    
#define VELOCIDADE_MAXIMA_GOTA 280.0f
#define GRAVIDADE_GOTA 900.0f          
#define RAIO_MINIMO_GOTA 2.0f
#define RAIO_MAXIMO_GOTA 6.0f
#define RAIO_MANCHA_SANGUE (TAMANHO_NOTA * 0.65f)
#define OPACIDADE_MANCHA_SANGUE 0.85f

#define TAMANHO_FONTE_INFO 24
#define TAMANHO_FONTE_FEEDBACK 56
#define TAMANHO_FONTE_TITULO 72
#define TAMANHO_FONTE_TEXTO 28

#define TAMANHO_PIP_DISTANCIA 18
#define ESPACO_PIP_DISTANCIA 6

#define CHAO_Y (ALTURA_CENA - 40)
#define SOBREVIVENTE_X 500
#define SOBREVIVENTE_LARGURA 70
#define SOBREVIVENTE_ALTURA 140
#define ASSASSINO_LARGURA 80
#define ASSASSINO_ALTURA 160
#define ASSASSINO_X_LONGE (MARGEM_TELA)
#define ASSASSINO_X_PERTO (SOBREVIVENTE_X - ASSASSINO_LARGURA)

#define ESPACO_LINHAS_REWIND 6
#define VELOCIDADE_LINHAS_REWIND 400.0f
#define OPACIDADE_ESCURECER_REWIND 0.35f
#define OPACIDADE_LINHAS_REWIND 0.1f

#define POSICAO_Y_TITULO_MENU 120
#define POSICAO_Y_OPCOES_MENU 320
#define ESPACO_LINHAS_MENU 50

#define LARGURA_COLUNA_MENU (LARGURA_JANELA - ALTURA_JANELA)
#define POSICAO_X_OPCOES_MENU 110
#define TAMANHO_FONTE_OPCOES_MENU 44
#define ESPACO_OPCOES_MENU 72
#define OPACIDADE_OPCAO_MENU 0.6f
#endif
