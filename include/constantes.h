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
#define PONTOS_POR_SEGUNDO_SEGURADO 200     // bonus da nota longa, por segundo segurado (x multiplicador)

//                      NOTAS LONGAS
#define TOLERANCIA_FIM_NOTA_LONGA 0.15f     // soltar ate 0,15 s antes do fim ainda conta como segurou ate o fim

//                        ASSASSINO
#define DISTANCIA_MINIMA_ASSASSINO 0
#define DISTANCIA_MAXIMA_ASSASSINO 4
#define DISTANCIA_INICIAL_ASSASSINO (DISTANCIA_MAXIMA_ASSASSINO)
#define SUAVIZACAO_ASSASSINO 6.0f            // quanto maior, mais rapido o assassino chega na posicao nova (~0,5 s)
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
#define AMOSTRAS_PASSO_GERADOR 256          // as janelas andam de ~5 ms em ~5 ms (uma sobrepoe a outra)
#define SEGUNDOS_HISTORICO_GERADOR 1.0f     // historico usado na media de energia
#define ENERGIA_MINIMA_BATIDA 0.0005f       // abaixo disso e silencio, nunca vira nota
#define SENSIBILIDADE_FACIL 1.6f            // quantas vezes acima da media para virar nota
#define SENSIBILIDADE_MEDIO 1.4f
#define SENSIBILIDADE_DIFICIL 1.25f
#define INTERVALO_MINIMO_FACIL 0.7f         // segundos entre duas notas
#define INTERVALO_MINIMO_MEDIO 0.28f
#define INTERVALO_MINIMO_DIFICIL 0.16f
#define FATOR_ACORDE_DIFICIL 3.0f           // batida muito forte no dificil vira acorde (nunca dois seguidos)

// Alinhamento das notas na batida da musica
#define PERIODO_MINIMO_BATIDA 0.30f         // segundos entre batidas: 200 bpm
#define PERIODO_MAXIMO_BATIDA 1.00f         // 60 bpm
#define VARIACAO_BUSCA_PERIODO 0.02f        // refina o periodo ate 2% para mais ou para menos
#define PASSOS_BUSCA_PERIODO 400            // quantos periodos testar nesse refinamento
#define DIVISOES_POR_BATIDA 2               // a nota pode cair na batida ou na meia batida
#define TOLERANCIA_GRADE 0.035f             // nota a mais de 35 ms da batida fica onde esta
#define MINIMO_NOTAS_PARA_GRADE 8           // com poucas notas nao da para achar o ritmo

//            DETECTOR DE MELODIA (fluxo espectral, ver melodia.c)
#define TAXA_AMOSTRAGEM_MELODIA 11025       // a melodia vai so ate 4000 Hz: analisar com menos amostras e mais rapido
#define TAMANHO_FFT_MELODIA 512             // potencia de 2; ~46 ms de som em 11025 Hz
#define PASSO_FFT_MELODIA 128               // ~12 ms entre uma analise e a proxima
#define FREQUENCIA_MINIMA_MELODIA 250.0f    // abaixo disso ficam bumbo e baixo
#define FREQUENCIA_MAXIMA_MELODIA 4000.0f   // acima disso ficam pratos e chiado
#define COMPRESSAO_ESPECTRO 100.0f          // log(1 + C*x): aproxima sons fortes e fracos
#define RAIO_MEDIANA_TEMPO 4                // separacao nota/batida: 4 analises para cada lado (~45 ms)
#define RAIO_MEDIANA_FREQUENCIA 4           // e 4 faixas para cada lado (~90 Hz)
#define SEGUNDOS_MEDIA_FLUXO 0.10f          // vizinhanca usada na media local do fluxo
#define SEGUNDOS_PICO_FLUXO 0.05f           // o pico precisa ser o maior nessa vizinhanca
#define FATOR_LIMIAR_MELODIA 1.15f           // o pico precisa passar 1,15x a media local...
#define LIMIAR_MINIMO_MELODIA 0.2f          // ...mais 0,2 da media da musica toda
#define ANALISES_ATE_ATAQUE 1               // correcao do tempo: o ataque fica ~1 passo depois do pico
#define ANALISES_ATE_ALTURA 2               // mede a altura da nota ~20 ms depois do ataque
#define PESO_SEGUNDO_HARMONICO 0.5f         // soma de harmonicos para achar a nota fundamental
#define PESO_TERCEIRO_HARMONICO 0.33f
#define FRACAO_SUSTENTACAO 0.6f             // a nota acaba quando a faixa dela cai abaixo de 60%
#define FREQUENCIA_LA_440 440.0f
#define NUMERO_NOTA_LA_440 69.0f
#define SEMITONS_POR_OITAVA 12.0f

// Escolha das notas da melodia em cada dificuldade
#define PERCENTIL_REFERENCIA_FORCA 0.9f     // forca de referencia: a nota que fica entre as 10% mais marcadas
#define FORCA_MINIMA_FACIL 0.35f            // x forca de referencia: no facil so as notas bem marcadas
#define FORCA_MINIMA_MEDIO 0.15f
#define FORCA_MINIMA_DIFICIL 0.08f
#define DENSIDADE_MINIMA_FACIL 0.5f         // notas por segundo: com menos que isso so de melodia, as batidas completam
#define DENSIDADE_MINIMA_MEDIO 1.0f
#define MINIMO_NOTAS_MELODIA 30             // musica com menos que isso (so percussao): usa as batidas
#define TOLERANCIA_MESMA_ALTURA 0.5f        // semitons: nota repetida fica na mesma lane
#define DURACAO_MINIMA_NOTA_LONGA 0.40f     // abaixo disso a nota e curta (duracao 0)
#define DURACAO_MAXIMA_NOTA_LONGA 3.0f
#define FOLGA_ANTES_PROXIMA_NOTA 0.10f      // a nota longa termina antes da proxima nota
#define FRACAO_FORCA_ENGOLIDA 0.5f          // nota com menos da metade da forca de uma nota longa, dentro dela, some

//                        INTERFACE
#define MARGEM_TELA 20
#define ESPESSURA_LINHA_ACERTO 4
#define ESPESSURA_BORDA_NOTA 5
#define OPACIDADE_NOTA_ERRADA 0.3f
#define ALTURA_RASTRO_NOTA (TAMANHO_NOTA / 2)
#define OPACIDADE_RASTRO_NOTA 0.55f

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

//                    HUD DA PARTIDA
#define CAMINHO_ICONE_REWIND "assets/icone_rewind.png"
#define CAMINHO_SPRITE_ASSASSINO "assets/sprites/Sprite_killer.png"
#define QUADROS_SPRITE_ASSASSINO 6         // a imagem tem 6 poses de corrida lado a lado
#define QUADROS_POR_SEGUNDO_ASSASSINO 10.0f
#define TRANSPARENCIA_MINIMA_SPRITE 0.1f    // pixel com menos opacidade que isso conta como fundo vazio
#define TAMANHO_FONTE_PONTOS 32
#define LARGURA_BARRA_PROGRESSO 660
#define ALTURA_BARRA_PROGRESSO 20
#define ESCALA_ICONE_REWIND 3           // icone de 16x14 pixels desenhado 3x maior
#define ESPACO_ICONES_REWIND 12
#define OPACIDADE_REWIND_GASTO 0.25f

#define CHAO_Y (ALTURA_CENA - 40)
#define SOBREVIVENTE_X 500
#define SOBREVIVENTE_LARGURA 70
#define SOBREVIVENTE_ALTURA 140
#define ASSASSINO_LARGURA 80
#define ASSASSINO_ALTURA 160
#define ASSASSINO_X_LONGE (MARGEM_TELA)
// O ponto "perto" do assassino e calculado na interface: SOBREVIVENTE_X - largura do sprite

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
