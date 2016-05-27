#include "estrutura.h"
#include "auxiliares.h"
#include "html.h"
#include "bots.h"

/** \brief Game Lobby a imprimir
 
 @param data    Estrutura atual
*/
void Game_Lobby(DATABASE data){
    switch (data.play) {
        case 0:
            imprime(data);
            break;
            
        case 1:
            imprime_start(data);
            break;
            
        case 4:
            imprime_fim(&data);
            break;
            
        default: // data.play == 2 -> jogo normal. data.play == 3 -> inicio do jogo. 4 -> fim do jogo
            jogo(&data);
            if(data.play != 4)
            	imprime(data);
    }
}
/** \brief Consoante a query do jogo, decide o consequente estado do jogo
 
 @param query   Query atual
*/
void parse (char * query) {
    DATABASE data = {{0},0,{0},0,0,0,0,{0},{0}};
    
    if(query!=NULL && strlen(query) != 0)
        data = STR2DATA(query);
    
    else{
        distribui(&data);
        data.play = 1;
    }
    Game_Lobby(data);
}

int main() {
    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<header><title>Big Two</title></header>\n");
    printf("<body>\n");
    srand(time(NULL));
    parse(getenv("QUERY_STRING"));
    printf("</body>\n");
    return 0;
}

