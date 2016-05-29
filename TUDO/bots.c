#include "estrutura.h"
#include "auxiliares.h"
#include "html.h"
/** \brief Jogada em que o bot não começa a ronda
 
 @param data    Estrutura atual
 @param m       Número do bot que vai jogar
*/
void bot_continua(DATABASE *data,int m){
    int draw, total, i, n, v;
    int jogadas[15][5]; //Primeiro elemento do array => número da jogada_possível; o 15 em vez de 13 é só por segurança. Segundo elemento do array => cada uma das cartas da jogada_possível. Exemplo: jogadas[0][0] e jogadas[0][1] dão a primeira jogada possível para uma jogada de duas cartas, jogadas[1][0] e jogadas[1][1] dão a segunda, ...
    int y[3];
    
    data->jogadas[m] = 0;
    if(data->nc!=5) {
        total = jogadas_possiveis(data,m,jogadas);
        if(total != 0){
            draw = rand() % total;
            for(i = 0; i < data->nc; i++){
                n = jogadas[draw][i] / 13;
                v = jogadas[draw][i] % 13;
                data->jogadas[m] = add_carta(data->jogadas[m],n,v);
                data->mao[m] = rem_carta(data->mao[m],n,v);
            }
        }
    }
    else {
        joga5(data,m);
        if(data->jogadas[m] != 0){
            for(i=0; i<52; i++){
                n = i/13;
                v = i%13;
                if(carta_existe(data->jogadas[m], n, v))
                    data->mao[m] = rem_carta(data->mao[m], n, v);
            }
            preenchejogada(data->jogadas[m],y);
            data->combination[0] = y[0];
            data->combination[1] = y[1];
            data->combination[2] = y[2];
        }
    }
    if(data->jogadas[m]!=0){
        data->passar = 0;
    }
    else {
        data->passar++;
    }
    if(data->mao[m]==0)
        data->play = 4;
}

/** \brief Bot a começar uma ronda
 
 @param data    Estrutura atual
 @param m       Número do bot que vai jogar
*/
void bot_comeca(DATABASE *data,int m){
    int total = 0, i; //total => número total de jogadas possíveis
    int jogadas[15][5];
    int draw;
    int n, v;
    int y[3];
    
    data->jogadas[m]=0;
    data->passar = 0;
    data->nc = 5; //para começar a testar combinações de 5 primeiro
    data->combination[0] = 0;
    data->combination[1] = 0;
    data->combination[2] = 0;
    if(data->play!= 3){
        if(joga5(data,m) != 0){
            for(i=0; i<52; i++){
                n = i/13;
                v = i%13;
                if(carta_existe(data->jogadas[m], n, v))
                    data->mao[m] = rem_carta(data->mao[m], n, v);
            }
            preenchejogada(data->jogadas[m],y);
            data->combination[0] = y[0];
            data->combination[1] = y[1];
            data->combination[2] = y[2];
        }
        else{
             data->nc = 4;
             while(total == 0){
                data->nc --;
                total = jogadas_possiveis(data, m, jogadas); //NOTA: já sai com o data->nc certo, é para isso que se tem o outro data->nc antes do while.
             }
                                    
              draw = rand()%total;
              for(i = 0; i < data->nc; i++){
                    n = jogadas[draw][i] / 13;
                    v = jogadas[draw][i] % 13;
                    data->jogadas[m] = add_carta(data->jogadas[m],n,v);
                    data->mao[m] = rem_carta(data->mao[m],n,v);
              }
        }
    }
    else{
        primeira_jogada(data, m);
        for(i=0; i<52; i++){
            n = i/13;
            v = i%13;
            if(carta_existe(data->jogadas[m], n, v))
                data->mao[m] = rem_carta(data->mao[m], n, v);
        }
        if(data->nc == 5){
            preenchejogada(data->jogadas[m],y);
            data->combination[0] = y[0];
            data->combination[1] = y[1];
            data->combination[2] = y[2];
        }
    }
    data->play = 2;
    if(data->mao[m]==0)
        data->play = 4;
}

/** \brief Responsável pela jogada dos Bots
 
 @param data    Estrutura atual
 @param m       Número do bot que vai jogar
*/
void joga_bots(DATABASE *data,int m){
    if(data->passar < 3)
        bot_continua(data,m);
    else
        bot_comeca(data,m);
}

/** \brief Responsável pelo jogo em geral
 
 @param data    Estrutura atual
*/
void jogo(DATABASE *data){
    int jog;
    if(data->play != 3)
        for(jog = 1; jog < 4; jog++){
    		joga_bots(data,jog);
            if(data->play == 4){
                imprime_fim(data);
            	break;
            }
        }
    else{ //só acontece no início do jogo se um dos bots tiver o 3 de ouros
        jog = quem_comeca(data);
        data -> passar = 3;
        if(jog!=0)
        	for(; jog < 4; jog++)
            	joga_bots(data,jog);
    }
}

