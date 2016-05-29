#include "estrutura.h"
#include "auxiliares.h"
#include "html.h"
#include "bots.h"

/** \brief Determina o que fazer na situação atual. As possibilidades são: imprimir imagem de início de jogo; imprimir o estado atual do jogo ; imprimir imagem de fim de jogo; simular uma ronda de jogo e imprimir o novo estado do jogo.
 
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
            
        default:
            jogo(&data);
            if(data.play != 4)
            	imprime(data);
    }
}
/** \brief Consoante a query do jogo, decide se se deve distribuir as cartas pelas mãos dos jogadores ou ir buscá-las à query
 
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

/** \brief Imprime os cabeçalhos de html iniciais e finais, fornece um valor para o "randomizer" e vai buscar a query do estado do jogo, que é passada para a função seguinte
 @return	Devolve 0 para indicar que o programa foi concluído com sucesso
 */
int main() {
    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<header><title>Big Two</title></header>\n");
    printf("<body>\n");
    srand(time(NULL));
    parse(getenv("QUERY_STRING"));
    printf("</body>\n");
    return 0;
}

