#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 URL da CGI
 */
#define SCRIPT		"http://127.0.0.1/cgi-bin/cartas"

/**
 URL da pasta com as cartas
 */
#define BARALHO		"http://127.0.0.1/cards"

/**
 Ordem dos naipes
 */
#define NAIPES		"DCHS"

/**
 Ordem das cartas
 */
#define VALORES		"3456789TJQKA2"

/**
 Formato da query string
 */
#define DATA "%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d"

/**
Tipo para a mão de um jogador
 */
typedef long long int MAO;

/**
 @brief Estrutura de dados com as informações do estado do jogo atual.
 */
struct database{
    MAO mao[4];         /**< Mao de cada jogador */
    MAO selected;       /**< Cartas selecionadas */
    MAO jogadas[4];     /**< Cartas da ultima jogada de cada jogador */
    int play;           /**< Identificador do tipo de jogo: 1 ->Começar;  2 ->Jogo normal; 3 ->Inicio do jogo; 4 ->Fim do jogo */
    int nc;             /**< Número de cartas da ronda */
    int passar;         /**< Contagem de passos */
    int ordenar;        /**< Identificador do tipo de ordenamento de cartas */
    int score[4];       /**< Pontuação de cada jogador */
    int combination[3]; /**< Posição 0: Tipo de combinação de 5 cartas. Posição 1: Valor da maior carta. Posição 2: Naipe da maior carta */
};

/**
 Renomeação da estrutura, para simplificar a sua denominação
 */
typedef struct database DATABASE;
