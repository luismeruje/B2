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

typedef long long int MAO;
struct database{
    MAO mao[4];
    MAO selected;
    MAO jogadas;
};
typedef struct database DATABASE;

//database STR2ESTADO (char *query){
//    database data = {{1},0};
//    return data;
//}
/** \brief Devolve o índice da carta
 
 @param naipe	O naipe da carta (inteiro entre 0 e 3)
 @param valor	O valor da carta (inteiro entre 0 e 12)
 @return		O índice correspondente à carta
 */
int indice(int naipe, int valor) {
    return naipe * 13 + valor;
}

/** \brief Adiciona uma carta a um dos estados
 
 @param ESTADO	O estado atual
 @param naipe	O naipe da carta (inteiro entre 0 e 3)
 @param valor	O valor da carta (inteiro entre 0 e 12)
 @return		O novo estado
 */
long long int add_carta(MAO ESTADO, int naipe, int valor) {
    int idx = indice(naipe, valor);
    return ESTADO | ((long long int) 1 << idx);
}

/** \brief Remove uma carta a um dos estados
 
 @param ESTADO	O estado atual
 @param naipe	O naipe da carta (inteiro entre 0 e 3)
 @param valor	O valor da carta (inteiro entre 0 e 12)
 @return		O novo estado
 */
long long int rem_carta(MAO ESTADO, int naipe, int valor) {
    int idx = indice(naipe, valor);
    return ESTADO & ~((long long int) 1 << idx);
}

/** \brief Verifica se uma carta pertence a um dos estados
 
 @param ESTADO	O estado atual
 @param naipe	O naipe da carta (inteiro entre 0 e 3)
 @param valor	O valor da carta (inteiro entre 0 e 12)
 @return		1 se a carta existe e 0 caso contrário
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
    sprintf(str,"%lld_%lld_%lld_%lld_%lld_%lld",data.mao[0],data.mao[1],data.mao[2],data.mao[3],data.selected,data.jogadas);//TODO: a string com os lld's acho que dá para subsituir por uma palavra com um "define" no topo
}

//Dá a string que fica no link do botao play
void DATA2STR_botao(char * str, DATABASE data){
    int ind,n,v,i,x,y;
    for(ind=0;ind<52;ind++)
        if(carta_existe2(data.selected,ind)){
            for(i=0;i<4;i++){
                n = ind/13;
                v = ind%13;
                if(carta_existe2(data.mao[i],ind)){
                    data.mao[i]=rem_carta(data.mao[i],n,v);
                    data.jogadas = add_carta(data.jogadas,n,v);
                }
            }
        }
    data.selected = 0;
    sprintf(str,"%lld_%lld_%lld_%lld_%lld_%lld",data.mao[0],data.mao[1],data.mao[2],data.mao[3],data.selected,data.jogadas);
}

//Passa a string que recebemos do browser para a nossa estrutura
DATABASE STR2DATA(char * str){
    DATABASE data;
    sscanf(str, "%lld_%lld_%lld_%lld_%lld_%lld",&(data.mao[0]),&(data.mao[1]),&(data.mao[2]),&(data.mao[3]),&(data.selected),&(data.jogadas));
    return data;
}


/** \brief Distribui as cartas pelas mãos de cada um dos jogadores, e põe o estado das cartas selecionadas tudo a 0's
 @return   Estrutura com as cartas distribuídas pelos jogadores
*/
DATABASE distribui(){
    int maoCount[4]={0};
    DATABASE data={{0},0};
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
 
 @param path	o URL correspondente à pasta que contém todas as cartas
 @param x A coordenada x da carta
 @param y A coordenada y da carta
 @param ESTADO	O estado atual
 @param naipe	O naipe da carta (inteiro entre 0 e 3)
 @param valor	O valor da carta (inteiro entre 0 e 12)
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
void imprime_play (char * path, DATABASE data){//meter para if combinação válida imprimir este link, senão meter só uma imagem do botão "desvanecido"
    //WARNING: não sei se no futuro, alterar data aqui não altera data na função imprim_carta, mas como está de ambas as maneiras é igual.
    char script [52000];
    DATA2STR_botao(script,data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"600\" y = \"450\" height = \"60\" width = \"30\" xlink:href = \"%s/botao.svg\" /></a>\n", script, path);
}

//Imprime o botão passar, que para já só mete as cartas seleecionadas a 0.
void imprime_passar (char * path, DATABASE data){
    char script[52000];
    data.selected = 0;
    sprintf(script,"%lld_%lld_%lld_%lld_%lld_%lld",data.mao[0],data.mao[1],data.mao[2],data.mao[3],data.selected,data.jogadas);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"650\" y = \"450\" height = \"60\" width = \"30\" xlink:href = \"%s/botao.svg\" /></a>\n", script, path);
}

/** \brief Imprime o estado
 
 Esta função está a imprimir o estado em quatro colunas: uma para cada naipe
 @param path	o URL correspondente à pasta que contém todas as cartas
 @param data	O estado atual
 */
void imprime(char *path, DATABASE data) {
    int n, v, p;
    int x, y,ind;
    
    printf("<svg height = \"800\" width = \"800\">\n");
    printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");
    //if(data.jogadas != 0)
    //    joga_bots;
    for(y = 10, p = 0; p < 2; p++, y += 420) {
        for(x = 200, ind = 0; ind < 52; ind++){
            if(p==1){
                if(carta_existe2(data.selected,ind))//desvio das cartas selecionadas
                    y -= 20;
                if(carta_existe2(data.mao[1],ind)){
                    n = ind/13;
                    v = ind%13;
                    imprime_carta(path,x,y,data,n,v);
                    x += 20;
                }
                if(carta_existe2(data.selected,ind))//anula o desvio, para n ficarem todas levantadas
                    y += 20;
            }
            else
                if(carta_existe2(data.mao[p],ind)){
                    n = ind/13;
                    v = ind%13;
                    imprime_carta_bot(path,x,y,n,v);
                    x += 20;
                }
                
        }
    }
    for(x = 10, p = 2; p < 4; p++, x += 600) {
        for(y = 60, ind = 0; ind < 52; ind++){
            if(carta_existe2(data.mao[p],ind)){
                n = ind/13;
                v = ind%13;
                imprime_carta_bot(path,x,y,n,v);
                y += 20;
            }
        }
    }
    for(x=200,y=200,ind=0;ind<52;ind++){
        if(carta_existe2(data.jogadas,ind)){
                n = ind/13;
                v = ind%13;
                imprime_carta_bot(path,x,y,n,v);
                x +=20;
        }
    }
    imprime_play(path,data);
    imprime_passar(path,data);
    printf("</svg>\n");
}

/** \brief Trata os argumentos da CGI
 
 Esta função recebe a query que é passada à cgi-bin e trata-a.
 Neste momento, a query contém o estado que é um inteiro que representa um conjunto de cartas.
 Cada carta corresponde a um bit que está a 1 se essa carta está no conjunto e a 0 caso contrário.
 Caso não seja passado nada à cgi-bin, ela assume que todas as cartas estão presentes.
 @param query A query que é passada à cgi-bin
 */
void parse (char * query) {
    DATABASE data = {{0},007700,0};
    if(query!=NULL && strlen(query) != 0) //n sei para q é preciso a primeira condição...
        data = STR2DATA(query);
    else
        data = distribui();
    imprime(BARALHO, data);
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