#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 URL da CGI
 */
#define SCRIPT      "http://127.0.0.1/cgi-bin/cartas"
/**
 URL da pasta com as cartas
 */
#define BARALHO     "http://127.0.0.1/cards"

/**
 Ordem dos naipes
 */
#define NAIPES      "DCHS"
/**
 Ordem das cartas
 */
#define VALORES     "3456789TJQKA2"

#define DATA "%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%d_%d_%d_%d_%d_%d_%d"

typedef long long int MAO;
struct database{
    MAO mao[4];
    MAO selected;
    MAO jogadas[4];
    int play;
    int nc; //número de cartas a serem jogadas por cada jogador
    int passar[4];
    int inicio;
};
typedef struct database DATABASE;

//database STR2ESTADO (char *query){
//    database data = {{1},0};
//    return data;
//}
/** \brief Devolve o índice da carta
 
 @param naipe   O naipe da carta (inteiro entre 0 e 3)
 @param valor   O valor da carta (inteiro entre 0 e 12)
 @return        O índice correspondente à carta
 */
int indice(int naipe, int valor) {
    return naipe * 13 + valor;
}

/** \brief Adiciona uma carta a um dos estados
 
 @param ESTADO  O estado atual
 @param naipe   O naipe da carta (inteiro entre 0 e 3)
 @param valor   O valor da carta (inteiro entre 0 e 12)
 @return        O novo estado
 */
long long int add_carta(MAO ESTADO, int naipe, int valor) {
    int idx = indice(naipe, valor);
    return ESTADO | ((long long int) 1 << idx);
}

/** \brief Remove uma carta a um dos estados
 
 @param ESTADO  O estado atual
 @param naipe   O naipe da carta (inteiro entre 0 e 3)
 @param valor   O valor da carta (inteiro entre 0 e 12)
 @return        O novo estado
 */
long long int rem_carta(MAO ESTADO, int naipe, int valor) {
    int idx = indice(naipe, valor);
    return ESTADO & ~((long long int) 1 << idx);
}

/** \brief Verifica se uma carta pertence a um dos estados
 
 @param ESTADO  O estado atual
 @param naipe   O naipe da carta (inteiro entre 0 e 3)
 @param valor   O valor da carta (inteiro entre 0 e 12)
 @return        1 se a carta existe e 0 caso contrário
 */
int carta_existe(MAO ESTADO, int naipe, int valor) {
    int idx = indice(naipe, valor);
    return (ESTADO >> idx) & 1;
}

/** \brief Verifica se uma carta existe num dos estados a partir do seu indice
 
*/

//Faz o mesmo que carta existe, mas usa o índice da carta em vez do naipe e do valor
int carta_existe2 (MAO mao,int idx){
    return (mao >> idx) & 1;
}

//Dá as strings que ficam nos links das nossas cartas
void DATA2STR(char * str,DATABASE data, int n, int v){
    if(carta_existe (data.selected, n,v)==0)
        data.selected = add_carta(data.selected,n,v);
    else
        data.selected = rem_carta(data.selected,n,v);
    data.play = 0;
    sprintf(str,DATA,data.mao[0],data.mao[1],data.mao[2],data.mao[3],data.selected,data.jogadas[0],data.jogadas[1],data.jogadas[2],data.jogadas[3],data.play,data.nc,data.passar[0],data.passar[1],data.passar[2],data.passar[3],data.inicio);//TODO: a string com os lld's acho que dá para subsituir por uma palavra com um "define" no topo
}

//Dá a string que fica no link do botao play
//TODO:fazer com que determine o data.nc
void DATA2STR_botao(char * str, DATABASE data){
    int ind,n,v,i;
    for(ind=0;ind<52;ind++)
        if(carta_existe2(data.selected,ind)){
            n = ind/13;
            v = ind%13;
            data.mao[0]=rem_carta(data.mao[0],n,v);
            data.jogadas[0] = add_carta(data.jogadas[0],n,v);
        }
    data.selected = 0;
    data.play = 1;
    for(i=0;i<4;i++){
        data.passar[i]=0;
    }
    sprintf(str,DATA,data.mao[0],data.mao[1],data.mao[2],data.mao[3],data.selected,data.jogadas[0],data.jogadas[1],data.jogadas[2],data.jogadas[3],data.play,data.nc,data.passar[0],data.passar[1],data.passar[2],data.passar[3],data.inicio);
}

//Passa a string que recebemos do browser para a nossa estrutura
DATABASE STR2DATA(char * str){
    DATABASE data;
    sscanf(str, DATA,&(data.mao[0]),&(data.mao[1]),&(data.mao[2]),&(data.mao[3]),&(data.selected),&(data.jogadas[0]),&(data.jogadas[1]),&(data.jogadas[2]),&(data.jogadas[3]),&data.play,&data.nc,&data.passar[0],&data.passar[1],&data.passar[2],&data.passar[3],&data.inicio);
    return data;
}


/** \brief Distribui as cartas pelas mãos de cada um dos jogadores, e põe o estado das cartas selecionadas tudo a 0's
 @return   Estrutura com as cartas distribuídas pelos jogadores
*/
DATABASE distribui(DATABASE data){
    int maoCount[4]={0};
    int n,v,j;
    for(n=0;n<4;n++){
        v = 0;
        while(v<13){
            j = rand()%4;
            if (maoCount[j]<13){
                data.mao[j] = add_carta(data.mao[j],n,v);
                maoCount[j]++;
                v++;
            }
        }
    }
    return data;
}

/** \brief Imprime o html correspondente a uma carta
 
 @param path    o URL correspondente à pasta que contém todas as cartas
 @param x A coordenada x da carta
 @param y A coordenada y da carta
 @param ESTADO  O estado atual
 @param naipe   O naipe da carta (inteiro entre 0 e 3)
 @param valor   O valor da carta (inteiro entre 0 e 12)
 */
void imprime_carta(char *path, int x, int y, DATABASE data, int naipe, int valor) {
    char *suit = NAIPES;
    char *rank = VALORES;
    char script[52000]; // n sei quanto precisamos.
    DATA2STR(script,data,naipe,valor);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" /></a>\n", script, x, y, path, rank[valor], suit[naipe]);
}

//Faz o mesmo que o imprime carta, mas só imprime uma imagem, em vez de um link com uma imagem
void imprime_carta_bot(char * path, int x, int y, int naipe, int valor){
    char *suit = NAIPES;
    char *rank = VALORES;
    printf("<image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" />\n", x, y, path, rank[valor], suit[naipe]);
}


//temos que meter os botões num ficheiro para os stores
//Imprime o botão play, que para já só apaga as cartas selecionadas das respetivas mãos
void imprime_play (char * path, DATABASE data){//TODO:meter para if combinação válida imprimir este link, senão meter só uma imagem do botão "desvanecido"
    //WARNING: não sei se no futuro, alterar data aqui não altera data na função imprim_carta, mas como está de ambas as maneiras é igual.
    //if(selecionadas são válidas para jogar)
    char script [52000];
    int i;
    data.jogadas[0]=0;
    DATA2STR_botao(script,data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"560\" y = \"460\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_play.svg\" /></a>\n", script, path);
    //else
    // imprimir só imagem do botão desvanecido
}

//Imprime o botão passar, que para já só mete as cartas seleecionadas a 0.
void imprime_passar (char * path, DATABASE data){
    char script[52000];
    int i,m;
    data.selected = 0;
    data.jogadas[0]=0;
    data.play = 1;
    data.passar[0]=1;
    sprintf(script,DATA,data.mao[0],data.mao[1],data.mao[2],data.mao[3],data.selected,data.jogadas[0],data.jogadas[1],data.jogadas[2],data.jogadas[3],data.play,data.nc,data.passar[0],data.passar[1],data.passar[2],data.passar[3],data.inicio);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"560\" y = \"500\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_pass.svg\" /></a>\n", script, path);
}


int maior_jogadas(MAO jogadas){
    int i, max=-1,n,v;
    for(i=0;i<52;i++)
        if(carta_existe2(jogadas,i)){
            v=i%13;
            n=i/13;
            if(v>max%13||(v==max%13&&n>max/13))
                max = i;
        }
    return max;
}
int all_passed(DATABASE data, int m){
    int i,c,result=0;
    for(i=0,c=0;i<4;i++)
        if(i!=m)
            if(data.passar[i]==1)
                c++;
    if(c==3)
        result = 1;
    return result;
}

DATABASE comeca_bot(DATABASE data,int m){
    int i,n,v;
    for(i=0;i<4;i++){
        data.passar[i]=0;
        data.jogadas[i]=0;
    }
    for(i=0;i<52;i++)
        if(carta_existe2(data.mao[m],i)){
            n=i/13;
            v=i%13;
            data.jogadas[m]=add_carta(data.jogadas[m],n,v);
            data.mao[m] = rem_carta(data.mao[m],n,v);
            break;
        }
    
    return data;
}

DATABASE joga_bots(DATABASE data,int m){
    int max,n,i,v,passou;
    for(i=0;i<4;i++){
        n=maior_jogadas(data.jogadas[i])/13;
        v=maior_jogadas(data.jogadas[i])%13;
        if(v>max%13||(v==max%13&&n>max/13))
            max = maior_jogadas(data.jogadas[i]);
    }
    data.jogadas[m]=0;
    //TODO: tem que testar, se os outros data.jogadas forem 0, pode jogar o que quiser, senão, faz o que está aqui em baixo, tem que se acrescentar à estrutura número da cartas a ser usado nesta jogada
    //retirar o break para dar com mais do que uma carta por jogador;
    if(all_passed(data,m)==0){
        passou=0;
        for(i=0;i<52;i++){
            n=i/13;
            v=i%13;
            if(carta_existe2(data.mao[m],i)==1)
                if(v>max%13||(v==max%13&&n>max/13)){
                    data.jogadas[m] = add_carta(data.jogadas[m],n,v);
                    data.mao[m] = rem_carta(data.mao[m],n,v);
                    passou ++;
                    break;
                }
        }
        if (passou == 0)
            data.passar[m]=1;
        else
            for(i=0;i<4;i++)
                data.passar[i]=0;
    }
    else
        data = comeca_bot(data,m);
    return data;
}

int quem_comeca(DATABASE data){
    int i,r=0;
    for(i=1;i<4;i++)
        if(carta_existe(data.mao[i],0,0)==1){
            r=i;
            break;
        }
    return r;
}

/** \brief Imprime o estado
 
 Esta função está a imprimir o estado em quatro colunas: uma para cada naipe
 @param path    o URL correspondente à pasta que contém todas as cartas
 @param data    O estado atual
 */
void imprime(char *path, DATABASE data) {
    int n, v, p;
    int x, y,ind;
    int i;
    
    printf("<svg height = \"800\" width = \"800\">\n");
    printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");
    if(data.inicio == 1){
        i = quem_comeca(data);
        if (i!=0)
            for(i;i<4;i++)
                data = joga_bots(data,i);
        data.inicio=2;
    }
    if(data.play == 1 && data.inicio==2)
        for(i=1;i<4;i++)
            data = joga_bots(data,i);
    for(y = 430, p = 0; p < 3; p+=2, y -= 420) {
        for(x = 200, ind = 0; ind < 52; ind++){
            if(p==0){
                if(carta_existe2(data.selected,ind)==1)//desvio das cartas selecionadas
                    y -= 20;
                if(carta_existe2(data.mao[p],ind)==1){
                    n = ind/13;
                    v = ind%13;
                    imprime_carta(path,x,y,data,n,v);
                    x += 20;
                }
                if(carta_existe2(data.selected,ind)==1)//anula o desvio, para n ficarem todas levantadas
                    y += 20;
            }
            else
                if(carta_existe2(data.mao[p],ind)==1){
                    n = ind/13;
                    v = ind%13;
                    imprime_carta_bot(path,x,y,n,v);
                    x += 20;
                }
                
        }
    }
    
    for(x = 10, p = 3; p > 0; p-=2, x += 650) {
        for(y = 100, ind = 0; ind < 52; ind++){
            if(carta_existe2(data.mao[p],ind)==1){
                n = ind/13;
                v = ind%13;
                imprime_carta_bot(path,x,y,n,v);
                y += 20;
            }
        }
    }
    for(x=300,y=150,i=2;i>=0;i-=2,y+=150){
        for(ind=0;ind<52;ind++)
            if(carta_existe2(data.jogadas[i],ind)){
                n = ind/13;
                v = ind%13;
                imprime_carta_bot(path,x,y,n,v);
                x +=20;
            }
        x=300;
    }
    
    
    for(x=150,y=200,i=3;i>0;i-=2,x+=350){
        for(ind=0;ind<52;ind++)
            if(carta_existe2(data.jogadas[i],ind)){
                n = ind/13;
                v = ind%13;
                imprime_carta_bot(path,x,y,n,v);
                y +=20;
            }
        y=200;
    }
    
    //data.jogadas = 0; //remover de data.jogadas a carta quando imprimida antes?
    imprime_play(path,data);
    imprime_passar(path,data);
    printf("</svg>\n");    
}

void check_finish(char * path, DATABASE data){
    int f=0,b;
    for(b=0;b<4;b++)
        if(data.mao[b]==0)
            f=1;
    if(f==1)
        printf("Fim\n");
    else
        imprime(path,data);
}

void imprime_start(DATABASE data,char * path){
    char script[52000];
    sprintf(script,DATA,data.mao[0],data.mao[1],data.mao[2],data.mao[3],data.selected,data.jogadas[0],data.jogadas[1],data.jogadas[2],data.jogadas[3],data.play,data.nc,data.passar[0],data.passar[1],data.passar[2],data.passar[3],data.inicio);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"350\" y = \"250\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_start.svg\" /></a>\n", script, path);
}

void start(char * path, DATABASE data){
    int y, x,p,ind,n,v;
    printf("<svg height = \"800\" width = \"800\">\n");
    printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");
    if(data.inicio==0){
        for(y = 430, p = 0; p < 3; p+=2, y -= 420)
            for(x = 200, ind = 0; ind < 52; ind++)
                if(carta_existe2(data.mao[p],ind)==1){
                    n = ind/13;
                    v = ind%13;
                    imprime_carta_bot(path,x,y,n,v);
                    x += 20;
                }
        for(x = 10, p = 3; p > 0; p-=2, x += 650)
            for(y = 100, ind = 0; ind < 52; ind++)
                if(carta_existe2(data.mao[p],ind)==1){
                    n = ind/13;
                    v = ind%13;
                    imprime_carta_bot(path,x,y,n,v);
                    y += 20;
                }
        data.inicio=1;
        imprime_start(data,path);
        printf("</svg>\n");
    }
    else
        imprime(path,data);
    
}
/** \brief Trata os argumentos da CGI
 
 Esta função recebe a query que é passada à cgi-bin e trata-a.
 Neste momento, a query contém o estado que é um inteiro que representa um conjunto de cartas.
 Cada carta corresponde a um bit que está a 1 se essa carta está no conjunto e a 0 caso contrário.
 Caso não seja passado nada à cgi-bin, ela assume que todas as cartas estão presentes.
 @param query A query que é passada à cgi-bin
 */
void parse (char * query) {
    DATABASE data = {{0},0,{0},0,0,{0},0};
    if(query!=NULL && strlen(query) != 0){ //n sei para q é preciso a primeira condição...
        data = STR2DATA(query);
        check_finish(BARALHO,data);
    }
    else{
        data = distribui(data); //é a primeira jogada, quem será que tem o precioso 3 de ouros?
        start(BARALHO, data);
    }
}


/** \brief Função principal
 
 Função principal do programa que imprime os cabeçalhos necessários e depois disso invoca
 a função que vai imprimir o código html para desenhar as cartas
 */
int main() {
    /*
     * Cabeçalhos necessários numa CGI
     */
    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<header><title>Exemplo</title></header>\n");
    printf("<body>\n");
    
    /*
     * Ler os valores passados à cgi que estão na variável ambiente e passá-los ao programa
     */
    srand(time(NULL));
    parse(getenv("QUERY_STRING"));
    printf("</body>\n");
    return 0;
}   

int calculascore(MAO mao){
    int ind,n,v,r=0;
    for (ind=0;ind<52;ind++){
        n = ind/13;
        v = ind%13;
        if(carta_existe(mao,n,v)==1) 
            r++:
    }
    if (r>9){
        if (r==13) r=39;
        else r=r*2;
    }
return r;
}