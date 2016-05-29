#include "estrutura.h"
#include "auxiliares.h"
/** \brief Imprime Carta como imagem
 
 @param x       Coordenada x
 @param y       Coordenada y
 @param n       Naipe
 @param v       Valor
*/
void imprime_carta_imagem(int x, int y,int n,int v){
    char *suit = NAIPES;
    char *rank = VALORES;
    printf("<image x = \"%d\" y = \"%d\" height = \"95\" width = \"70\" xlink:href = \"%s/%c%c.svg\" />\n", x, y, BARALHO, rank[v], suit[n]);
}

/** \brief Imprime Parte de trás da Carta como imagem
 
 @param x       Coordenada x
 @param y       Coordenada y
*/
void imprime_carta_back(int x, int y){
    printf("<image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/card_back.svg\" />\n", x, y, BARALHO);
}

/** \brief Imprime Carta como link
 
 @param x       Coordenada x
 @param y       Coordenada y
 @param data    Estrutura Atual
 @param n       Naipe
 @param v       Valor
*/
void imprime_carta_link(int x, int y,DATABASE data,int n,int v){
    char *suit = NAIPES;
    char *rank = VALORES;
    char script[1000];
    if(carta_existe(data.selected,n,v)==0)
        data.selected = add_carta(data.selected,n,v);
    else
        data.selected = rem_carta(data.selected,n,v);
    if (data.play!=3) data.play = 0;
    
    DATA2STR(script,data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" /></a>\n", script, x, y, BARALHO, rank[v], suit[n]);
}

/** \brief Imprime Cartas Jogadas
 
 @param data    Estrutura atual
*/
void imprime_jogadas(DATABASE data){
    int x,y;
    int n,v;
    int jog;
    int f = -1;
    
    for(jog = 1; jog < 4; jog++) {
        if(carta_existe(data.jogadas[jog], 0, 0)) f = jog;
    }
    if(carta_existe(data.mao[0], 0, 0)) f = 0;
    for(x=550,y=220,jog=2;jog>=0;jog-=2,y+=165){
        if (data.jogadas[jog]==0 && ((f==1 && jog!=0) || f==-1))  {
            if (data.play || jog!=0) {
                y -= 20;
                printf("<image x = \"%d\" y = \"%d\" height = \"100\" width = \"100\" xlink:href = \"%s/passo_%d.svg\" />\n", x, y, BARALHO, jog);
                y += 20;
            }
        }
        else {
            if (data.play!=0 || jog!=0)
                for(v=0; v<13; v++)
                    for(n=0;n<4;n++)
                        if(carta_existe(data.jogadas[jog],n,v)){
                            imprime_carta_imagem(x,y,n,v);
                            x +=20;
                        }
        }
        x=550;
    }
    for(x=360,y=250,jog=3;jog>0;jog-=2,x+=410){
        if (data.jogadas[jog]==0 && ((f>0 && jog!=1) || f==-1))
            printf("<image x = \"%d\" y = \"%d\" height = \"100\" width = \"100\" xlink:href = \"%s/passo_%d.svg\" />\n", x, y, BARALHO, jog);
        else
            for(v=0; v<13; v++)
                    for(n=0;n<4;n++)
                if(carta_existe(data.jogadas[jog],n,v)){
                    imprime_carta_imagem(x,y,n,v);
                    y +=20;
                }
        y=250;
    }
}

/** \brief Imprime Mãos dos jogadores
 
 @param data    Estrutura atual
*/
void imprime_maos (DATABASE data){
    int n,v,b,i;
    int x,y;
    int jog;
    int ind;
    
    if (data.ordenar) b = 13;
    else b = 1;
    for(y = 500, jog = 0; jog < 3; jog+=2, y -= 415)
        for(x = 445, ind = 0, i = 0; ind < 52; ind += b){
            n = ind/13;
            v = ind%13;
            if(jog==0){
                if(carta_existe(data.selected,n,v)==1)//desvia as cartas selecionadas
                    y -= 20;
                if(carta_existe(data.mao[jog],n,v)==1){
                    imprime_carta_link(x,y,data,n,v);
                    x += 20;
                }
                if(carta_existe(data.selected,n,v)==1)//anula o desvio, para não ficarem todas levantadas
                    y += 20;
            }
            else{
                n = ind/13;
                v = ind%13;
                if(carta_existe(data.mao[jog],n,v)==1){
                    imprime_carta_back(x,y);
                    x += 20;
                }
            }
            if (data.ordenar && ind > 38 && ind != 51) {
                i+=1;
                ind = i-13;
            }
        }
    for(x = 240, jog = 3; jog > 0; jog-=2, x += 640)
        for(y = 170, ind = 0, i = 0; ind < 52; ind += b){
            n = ind/13;
            v = ind%13;
            if(carta_existe(data.mao[jog],n,v)==1){
                imprime_carta_back(x,y);
                y += 20;
            }
            if (data.ordenar && ind > 38 && ind != 51) {
                i+=1;
                ind = i-13;
            }
        }
    
}

/** \brief Imprime Botão Ordenar
 
 @param data    Estrutura atual
*/
void botao_ordenar (DATABASE data){
    char script [1000];
    if (data.ordenar==0) {
        data.ordenar = 1;
        if(data.play!=3) data.play = 0;
        DATA2STR(script, data);
        printf("<a xlink:href = \"cartas?%s\"><image x = \"350\" y = \"510\" height = \"40\" width = \"40\" xlink:href = \"%s/baralhar_por_valor.svg\" /></a>\n", script, BARALHO);
    }
    else {
        data.ordenar = 0;
        if(data.play!=3) data.play = 0;
        DATA2STR(script, data);
        printf("<a xlink:href = \"cartas?%s\"><image x = \"350\" y = \"510\" height = \"40\" width = \"40\" xlink:href = \"%s/baralhar_por_naipe.svg\" /></a>\n", script, BARALHO);
    }
}

/** \brief Imprime Botão Help que ao clicar seleciona possível jogada
 
 @param data    Estrutura atual
*/
void botao_help(DATABASE * data){
    char script[1000];
    int jogadas[13][5];
    int total = 0;
    int n, v, draw, i;
    data->selected = 0;

    if(data->play!=3) {
        if(data->passar != 3){
            if ((data->nc) == 5) {
                joga5(data,0);
                data->selected = data->jogadas[0];
                data->jogadas[0] = 0;
            }
            else {
                jogadas_possiveis(data, 0, jogadas);
                draw = 0;
                for(i = 0; i < data->nc; i++){
                    n = jogadas[draw][i] / 13;
                    v = jogadas[draw][i] % 13;
                    data->selected = add_carta(data->selected,n,v);
                }
            }
        }
        else {
            data->nc = 5;
            total = joga5(data,0);
            if (total == 0){
                data->nc = 4;
                while(total == 0 && data->nc > 0){
                    data->nc--;
                    total = jogadas_possiveis(data, 0, jogadas);
                }
                draw = 0;
                for(i = 0; i < data->nc; i++){
                    n = jogadas[draw][i] / 13;
                    v = jogadas[draw][i] % 13;
                    data->selected = add_carta(data->selected,n,v);
                }
            }
            else {
                data->selected = data->jogadas[0];
                data->jogadas[0] = 0;
            }
        }
    }
    else {
        primeira_jogada(data, 0);
        data->selected = data->jogadas[0];
        data->jogadas[0] = 0;
    }
    if (data->play!=3) data->play = 0;
    DATA2STR(script,* data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"370\" y = \"560\" height = \"40\" width = \"40\" xlink:href = \"%s/botao_help.svg\" /></a>\n", script, BARALHO);
}

/** \brief Imprime Botão Passar
 
 @param data    Estrutura atual
*/
void botao_passar (DATABASE data){
    char script[1000];
    if(data.passar!=3) {
        data.selected = 0;
        data.jogadas[0]=0;
        data.play = 2;
        data.passar++;
        DATA2STR(script, data);
        printf("<a xlink:href = \"cartas?%s\"><image x = \"775\" y = \"550\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_pass.svg\" /></a>\n", script, BARALHO);
    }
    else printf("<image x = \"775\" y = \"550\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_pass_cinza.svg\" />\n", BARALHO);
}

/** \brief Imprime Botão Play
 
 @param data    Estrutura atual
*/
void botao_play (DATABASE data){
    int n, v, ind;
    int y[3];
    char script [1000];
    data.jogadas[0] = 0;
    if(data.passar == 3){
        data.nc = 0;
        for(ind=0;ind<52;ind++){
            n=ind/13;
            v=ind%13;
            if(carta_existe(data.selected,n,v)) data.nc ++;
        }
    }
    if(data.nc < 6 && check_jogada(&data,0)){
        for(ind=0;ind<52;ind++){
            n = ind/13;
            v = ind%13;
            if(carta_existe(data.selected,n,v)){
                data.mao[0]=rem_carta(data.mao[0],n,v);
                data.jogadas[0] = add_carta(data.jogadas[0],n,v);
            }
        }
        data.selected = 0;
        data.passar = 0;
        if(data.nc == 5){
            preenchejogada(data.jogadas[0],y);
            data.combination[0] = y[0];
            data.combination[1] = y[1];
            data.combination[2] = y[2];
        }
        if(data.mao[0] == 0) data.play = 4; //4 -> fim do jogo
        else data.play = 2; //2 -> jogo normal
        DATA2STR(script, data);
        printf("<a xlink:href = \"cartas?%s\"><image x = \"775\" y = \"510\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_play.svg\" /></a>\n", script, BARALHO);
    }
    else
        printf("<image x = \"775\" y = \"510\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_play_cinza.svg\" />\n", BARALHO);
}

/** \brief Imprime Botão Continuar
 
 @param data    Estrutura atual
*/
void botao_continuar(DATABASE * data) {
    int i;
    char script[1000];
    DATABASE newdata = {{0},0,{0},0,0,0,0,{0},{0}}; 
    
    for(i=0; i < 4; i++)
        newdata.score[i] = data->score[i];
    distribui(&newdata);
    newdata.play = 1;
    DATA2STR(script,newdata);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"380\" y = \"500\" height = \"60\" width = \"170\" xlink:href = \"%s/botao_continuar.svg\" /></a>\n", script, BARALHO);
}

/** \brief Imprime Botão Novo Jogo
 
 @param data    Estrutura atual
*/
void botao_novojogo() {
    printf("<a xlink:href = \"cartas\"><image x = \"630\" y = \"500\" height = \"60\" width = \"170\" xlink:href = \"%s/botao_novo_jogo.svg\" /></a>\n", BARALHO);
}


/** \brief Imprime Botão FIM
 
 @param data    Estrutura atual
*/
void imprime_fim (DATABASE *data){
    printf("<svg height = \"680\" width = \"1200\">\n");
    printf("<image x = \"-155\" y = \"0\" height = \"900\" width = \"1500\" xlink:href = \"%s/floor.svg\" />\n", BARALHO);
    printf("<circle cx=\"450\" cy=\"350\" r=\"290\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<circle cx=\"750\" cy=\"350\" r=\"290\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<rect x = \"450\" y = \"60\" height = \"580\" width = \"300\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<rect x = \"440\" y = \"70\" height = \"560\" width = \"320\" style = \"fill:#007700\"/>\n");
    data->score[0] += calcula_score(data->mao[0]);
    data->score[1] += calcula_score(data->mao[1]);
    data->score[2] += calcula_score(data->mao[2]);
    data->score[3] += calcula_score(data->mao[3]);
    printf("<text x = \"550\" y = \"170\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:55px;\">Fim</text>\n");
    printf("<text x = \"400\" y = \"240\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:45px;\">Pontuação:</text>\n");
    printf("<text x = \"400\" y = \"310\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">Jogador :</text>\n");
    printf("<text x = \"400\" y = \"360\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">Jorge :</text>\n");
    printf("<text x = \"400\" y = \"410\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">Luís :</text>\n");
    printf("<text x = \"400\" y = \"460\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">Diogo :</text>\n");
    printf("<text x = \"600\" y = \"310\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">%d</text>\n", data->score[0]);
    printf("<text x = \"600\" y = \"360\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">%d</text>\n", data->score[1]);
    printf("<text x = \"600\" y = \"410\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">%d</text>\n", data->score[2]);
    printf("<text x = \"600\" y = \"460\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">%d</text>\n", data->score[3]);
    botao_continuar(data);
    botao_novojogo();
    printf("</svg>");
}

/** \brief Imprime Botão Start
 
 @param data    Estrutura atual
*/
void botao_start(DATABASE data){
    char script[1000];
    
    DATA2STR(script, data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"510\" y = \"300\" height = \"60\" width = \"180\" xlink:href = \"%s/botao_start.svg\" /></a>\n", script, BARALHO);
}

/** \brief Imprime Começo de Jogo (Mesa, Chão...)
 
 @param data    Estrutura atual
*/
void imprime_start(DATABASE data){
    int y, x,p,ind;
    
    printf("<svg height = \"680\" width = \"1200\">\n");
    printf("<image x = \"-155\" y = \"0\" height = \"900\" width = \"1500\" xlink:href = \"%s/floor.svg\" />\n", BARALHO);
    printf("<circle cx=\"450\" cy=\"350\" r=\"290\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<circle cx=\"750\" cy=\"350\" r=\"290\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<rect x = \"450\" y = \"60\" height = \"580\" width = \"300\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<rect x = \"440\" y = \"70\" height = \"560\" width = \"320\" style = \"fill:#007700\"/>\n");
    for(y = 500, p = 0; p < 3; p+=2, y -= 415)
        for(x = 445, ind = 0; ind < 13; ind++){
            imprime_carta_back(x,y);
            x += 20;
        }
    for(x = 240, p = 3; p > 0; p-=2, x += 640)
        for(y = 170, ind = 0; ind < 13; ind++){
            imprime_carta_back(x,y);
            y += 20;
        }
    data.play = 3;
    botao_start(data);
    printf("</svg>\n");
}

/** \brief Imprime Jogo
 
 @param data    Estrutura atual
*/
void imprime (DATABASE data){
    printf("<svg height = \"680\" width = \"1200\">\n");
    printf("<image x = \"-155\" y = \"0\" height = \"900\" width = \"1500\" xlink:href = \"%s/floor.svg\" />\n", BARALHO);
    printf("<circle cx=\"450\" cy=\"350\" r=\"290\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<circle cx=\"750\" cy=\"350\" r=\"290\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<rect x = \"450\" y = \"60\" height = \"580\" width = \"300\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<rect x = \"440\" y = \"70\" height = \"560\" width = \"320\" style = \"fill:#007700\"/>\n");
    imprime_maos(data);
    imprime_jogadas(data);
    botao_play(data);
    botao_passar(data);
    botao_ordenar(data);
    botao_help(&data);
}
