#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*
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

#define DATA "%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d"

typedef long long int MAO;
struct database{
    MAO mao[4];
    MAO selected;
    MAO jogadas[4];
    int play;
    int nc; //número de cartas a serem jogadas por cada jogador
    int passar;
    int inicio;
    int baralhar; 
    int score[4];
    int combination[3];
};

typedef struct database DATABASE;

//#################################UTILITÁRIOS#############################################

DATABASE STR2DATA(char * str){
    DATABASE data;
    sscanf(str, DATA,&(data.mao[0]),&(data.mao[1]),&(data.mao[2]),&(data.mao[3]),&(data.selected),&(data.jogadas[0]),&(data.jogadas[1]),&(data.jogadas[2]),&(data.jogadas[3]),&data.play,&data.nc,&data.passar,&data.inicio,&data.baralhar,&data.score[0],&data.score[1],&data.score[2],&data.score[3],&data.combination[0],&data.combination[1],&data.combination[2]);
    return data;
}

void DATA2STR(char * str,DATABASE data){
    sprintf(str,DATA,data.mao[0],data.mao[1],data.mao[2],data.mao[3],data.selected,data.jogadas[0],data.jogadas[1],data.jogadas[2],data.jogadas[3],data.play,data.nc,data.passar,data.inicio,data.baralhar,data.score[0],data.score[1],data.score[2],data.score[3],data.combination[0],data.combination[1],data.combination[2]);
}
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

//WARNING: se metermos um count que faz a função parar aos 5 fica mais eficiente, não percorre as cartas todas
void separa_val (MAO ESTADO, int y[13]){
    int i,n,v;
    for(i=0;i<52;i++){
        n = i/13;
        v = i%13;
        if(carta_existe(ESTADO,n,v)==1){
            y[v]++;
        }
    }
}

//WARNING: se metermos um count que faz a função parar aos 5 fica mais eficiente, não percorre as cartas todas
void separa_nap (MAO ESTADO, int y[4]){
    int i,n,v;
    for(i=0;i<52;i++){
        n = i/13;
        v = i%13;
        if(carta_existe(ESTADO,n,v)==1){
            y[n]++;
        }
    }
}


//acho que vai ser util criar um array de 6 na estrutura ou assim para as jogadas de 5 que diz os indices das cartas jogadas e na a[5] metemos o rank da jogada (se é straight flush e assim)
int teste_straight(int v[13]){
    int r = 0, i, count = 0;
    for(i = 0;v[i] == 0; i++);
    for(; v[i] != 0 && count < 5; i+1 / 13)
        count++;
    if(count == 5)
        r = 1;
    return r;
}

int teste_flush(int naipe[4]) {
  int r = 0, n;
  for (n=0; n<4; n++) {
    if(naipe[n]==5) {
      r = 1;
      n = 4;
    }
  }
  return r;
}

int teste_fullhouse(int rank[13]) {
  int r = 1, v;
  for (v=0; v<13; v++) {
    if (rank[v]==4 || rank[v]==1) r = 0;
  }
  return r;
}

int teste_fourofakind(int rank[13]) {
  int r = 0, v;
  for (v=0; v<13; v++) {
    if (rank[v]==4) r = 1;
  }
  return r;
}

//r: 1 => straight; 2 => flush; 3 => fullhouse; 4 => fourofakind; 5 => straightflush
int tipo_comb_five(MAO mao) {
  int r = 0, c;
  int n[4]={0};
  int v[13] = {0};
  separa_nap(mao, n);
  separa_val(mao, v);
  if(teste_straight(v)) {
    if(teste_flush(n)) r = 5;
    else r = 1;
  }
  if(r==0 && teste_flush(n)) r = 2;
  if(r==0 && teste_fullhouse(v)) r = 3;
  if(r==0 && teste_fourofakind(v)) r = 4;
  return r;
} 

/* à priori vai ser a mais dificil de fazer de todas, para tratar amanha terca
int teste_straighflush(MAO mao){

}



int compstraight(MAO mao,DATABASE data){
  int i,max,p=1,teste[5];int
  for(ind=0;ind<52;ind++){
    n=ind/13;
    v=ind%13;
    if(carta_existe(data.selected,n,v)){
    teste[p]=i;  //o objetivo era criar uma funcao insere ordenado
    p++;
    }

int compflush (MAO mao,MAO mao){

}

int compFullHouse (MAO mao,MAO mao){

}

int compfourofakind (MAO mao,MAO mao){

}
*/

//Funcao que calcula a pontuacão de cada jogador
int calcula_score(MAO mao){
    int ind;
    int n,v;
    int r=0;

    for (ind=0;ind<52;ind++){
        n = ind/13;
        v = ind%13;
        if(carta_existe(mao,n,v)==1)
            r++;
    }
    if (r>9){
        if (r==13)
            r=39;
        else
            r=r*2;
    }
    return r;
}

int maior_carta_mao(MAO mao){
    int ind;
    int max=-1;
    int n,v;
    for(ind=0;ind<52;ind++){
        n=ind/13;
        v=ind%13;
        if(carta_existe(mao,n,v)){
            if(v>max%13||(v==max%13&&n>max/13))
                max = ind;
        }
    }
    return max;
}

int maior_carta_jogada(DATABASE data){
  int jog;
  int n,v;
  int temp;
  int max = -1;
  for(jog=0;jog<4;jog++){
    temp = maior_carta_mao(data.jogadas[jog]);
    n = temp/13;
    v = temp%13;
    if(v>max%13||(v==max%13&&n>max/13))
      max = temp;
  }
  return max;
}

void imprime_continuar(char *path, DATABASE data) {
  char script[52000];
  data.mao[0] = 0;
  data.mao[1] = 0;
  data.mao[2] = 0;
  data.mao[3] = 0;
  data.selected = 0;
  data.jogadas[0] = 0;
  data.jogadas[1] = 0;
  data.jogadas[2] = 0;
  data.jogadas[3] = 0;
  data.play = 0;
  data.nc = 0;
  data.passar = 0;
  data.inicio = 0;
  data.combination[0]=0;
  data.combination[1]=0;
  data.combination[2]=0;
  DATA2STR(script,data);
  printf("<a xlink:href = \"cartas?%s\"><image x = \"380\" y = \"500\" height = \"60\" width = \"170\" xlink:href = \"%s/botao_continuar.svg\" /></a>\n", script, path);
}

void imprime_novojogo(char *path) {
  printf("<a xlink:href = \"cartas?\"><image x = \"630\" y = \"500\" height = \"60\" width = \"170\" xlink:href = \"%s/botao_novo_jogo.svg\" /></a>\n", path);
}

void imprime_fim (char * path, DATABASE data){
    data.score[0] += calcula_score(data.mao[0]);
    data.score[1] += calcula_score(data.mao[1]);
    data.score[2] += calcula_score(data.mao[2]);
    data.score[3] += calcula_score(data.mao[3]);
    printf("<text x = \"550\" y = \"170\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:55px;\">Fim</text>\n");
    printf("<text x = \"400\" y = \"240\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:45px;\">Pontuação:</text>\n");
    printf("<text x = \"400\" y = \"310\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">Jogador - %d</text>\n", data.score[0]);
    printf("<text x = \"400\" y = \"350\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">Jorge - %d</text>\n", data.score[1]);
    printf("<text x = \"400\" y = \"390\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">Luís - %d</text>\n", data.score[2]);
    printf("<text x = \"400\" y = \"430\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">Diogo - %d</text>\n", data.score[3]);
    imprime_continuar(path, data);
    imprime_novojogo(path);
}


int all_passed(DATABASE data){
    int result=0;
    if (data.passar>=3)
        result=1;
    return result;
}

DATABASE check_cartas(int n, int v, DATABASE data,int m){
  int count;
  int naipe;
  int i = 0;
  DATABASE data1 = data;
  count = data.nc - 1;
  for(naipe = 0;naipe < 4;naipe++){
    if(carta_existe(data.mao[m],naipe,v)==1){
      i++;
    }
  }
  if(i >= data.nc){
    data.mao[m] = rem_carta(data.mao[m],n,v);
    data.jogadas[m]=add_carta(data.jogadas[m],n,v);
    for(naipe = 0;count>0&& naipe < 4; naipe++){
      if(carta_existe(data.mao[m],naipe,v)==1&&naipe != n){
        data.mao[m] = rem_carta(data.mao[m],naipe,v);
        data.jogadas[m] = add_carta(data.jogadas[m],naipe,v);
        count --;
      }
    }
  }
  return data;
}

void imprime_carta_imagem(char * path,int x, int y,int n,int v){
    char *suit = NAIPES;
    char *rank = VALORES;
    printf("<image x = \"%d\" y = \"%d\" height = \"95\" width = \"70\" xlink:href = \"%s/%c%c.svg\" />\n", x, y, path, rank[v], suit[n]);
}

void imprime_carta_back(char * path,int x, int y){
    printf("<image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/card_back.svg\" />\n", x, y, path);
}

void imprime_carta_link(char * path,int x, int y,DATABASE data,int n,int v){
    char *suit = NAIPES;
    char *rank = VALORES;
    char script[52000]; // n sei quanto precisamos.
    if(carta_existe(data.selected,n,v)==0)
        data.selected = add_carta(data.selected,n,v);
    else
        data.selected = rem_carta(data.selected,n,v);
    data.play = 0;
    //vale a pena data.jogadas[0]=0, para perceber quando ganha? acho que nao.
    DATA2STR(script,data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" /></a>\n", script, x, y, path, rank[v], suit[n]);
}

void imprime_help(DATABASE data, char * path){
    DATABASE help;
    MAO old = data.selected;
    char script[52000];
    int max = maior_carta_jogada(data);
    int ind;
    int n,v;
    data.jogadas[0] = 0;
    for(ind=0;ind<52;ind++){
      n=ind/13;
      v=ind%13;
      if(carta_existe(data.mao[0],n,v)==1)
        if(v>max%13||(v==max%13&&n>max/13)){
          help = check_cartas(n,v,data,0);
          if (help.jogadas[0]!=0) break;
        }
    }
    data.selected = 0;
    if(old != help.jogadas[0])
        data.selected = help.jogadas[0];
    data.play = 0;
    DATA2STR(script,data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"370\" y = \"560\" height = \"40\" width = \"40\" xlink:href = \"%s/botao_help.svg\" /><>\n", script, path);
}

int quem_comeca(DATABASE data){
    int first,jog;
    for(jog=0;;jog++)
        if(carta_existe(data.mao[jog],0,0)){
            first = jog;
            break;
        }
    return first;
}

//##############################UTILITÁRIOS-FIM################################################
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

//##########################Funções para imprimir#########################################

void imprime_start(DATABASE data,char * path){
    char script[52000];
    sprintf(script,DATA,data.mao[0],data.mao[1],data.mao[2],data.mao[3],data.selected,data.jogadas[0],data.jogadas[1],data.jogadas[2],data.jogadas[3],data.play,data.nc,data.passar,data.inicio,data.baralhar,data.score[0],data.score[1],data.score[2],data.score[3],data.combination[0],data.combination[1],data.combination[2]);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"510\" y = \"300\" height = \"60\" width = \"180\" xlink:href = \"%s/botao_start.svg\" /></a>\n", script, path);
}

void imprime_passar (DATABASE data,char * path){
    char script[52000];
  if (data.nc != 0 || all_passed(data) != 0) {
    data.selected = 0;
    data.jogadas[0]=0;
    data.play = 1;
    data.passar++;
  }
  DATA2STR(script, data);
  printf("<a xlink:href = \"cartas?%s\"><image x = \"775\" y = \"550\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_pass.svg\" /></a>\n", script, path);
}
// sendo assim, 1º elemento do array da rank da combinacao de 5 o seguinte dá o valor da carta e o ultimo dá o naipe da maior carta (em caso de straight e flush) 
// (no 4 of a kind da o valor da combinacao de 4) e (no full house o valor da carta que aparece 3 vezes... :))
// supostamente vamos meter o rank da play diretamente da funcao responsavel por jogar por isso esta so atualiza a posicao 1 e 2 
void atualizacomb (DATABASE data) {
  int ind, n, v, nmax=0,vmax=0;
    for(ind=0;ind<52;ind++){
      n=ind/13;
      v=ind%13;
    if(carta_existe(data.selected,n,v))
      if ((v > vmax) || (v == vmax && n > nmax)) {
        nmax = n;
        vmax = v;
      }
    }
      data.combination[1]= nmax;
      data.combination[2]= vmax;
}

//falta limitar com 4....
int pode_jogar(DATABASE data){
    int ind,n,v;
    int count=0;
    int r=0;
    int selec[5];
    int i = 0;
    int max;
    for(ind=0;ind<52;ind++){
        n=ind/13;
        v=ind%13;
        if(carta_existe(data.selected,n,v)){
            selec[i] = ind;
            count++;
            i++;
        }
    }
    if (count < 4){
    if(count==data.nc&&data.nc!=0){
          for(i=0;i<data.nc;i++){
             n=selec[i]/13;
             v=selec[i]%13;
             max = maior_carta_jogada(data);
             if(v>max%13||(v==max%13&&n>max/13))
                r=1;
          }
       }

    for(i=0; i<count; i++){
    n = selec[0]%13;
    if(selec[i]%13!=n) r=0;
    }
  }

    else if (count==5 && data.nc==5){
    if (tipo_comb_five(data.selected) >0) r=1;
  }
    return r;
}


void imprime_play (DATABASE data, char * path){
    int n,v;
    int ind;
    char script [52000];
    data.jogadas[0]=0;
    if(data.passar == 3||data.nc==0){
        data.nc = 0;
        for(ind=0;ind<52;ind++){
            n=ind/13;
            v=ind%13;
            if(carta_existe(data.selected,n,v))
                data.nc ++;
        }
    }
    if(pode_jogar(data)==1){
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
        if(data.mao[0] == 0)
            data.play = 2;
        else
            data.play = 1;
        DATA2STR(script, data);
        printf("<a xlink:href = \"cartas?%s\"><image x = \"775\" y = \"510\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_play.svg\" /></a>\n", script, path);
    }
    else
        printf("<image x = \"775\" y = \"510\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_play_cinza.svg\" />\n", path);
}

void imprime_baralhar (DATABASE data, char *path){
  char script [52000];
  if (data.baralhar==0) {
    data.baralhar = 1;
    data.play = 0;
    DATA2STR(script, data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"350\" y = \"510\" height = \"40\" width = \"40\" xlink:href = \"%s/baralhar_por_valor.svg\" /></a>\n", script, path);
  }
  else {
    data.baralhar = 0;
    data.play = 0;
    DATA2STR(script, data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"350\" y = \"510\" height = \"40\" width = \"40\" xlink:href = \"%s/baralhar_por_naipe.svg\" /></a>\n", script, path);
  }
}

void imprime_jogadas(DATABASE data, char * path){
    int x,y;
    int n,v;
    int jog;
    int ind;
    for(x=550,y=210,jog=2;jog>=0;jog-=2,y+=175){ //Pôr a ficar o jogador a somar?
      if (data.jogadas[jog]==0 && data.inicio!=1) {
        if (data.play || jog!=0) {
        y -= 20;
        printf("<image x = \"%d\" y = \"%d\" height = \"100\" width = \"100\" xlink:href = \"%s/passo_%d.svg\" />\n", x, y, path, jog);
        y += 20;
        }
      }
      else {
        if (data.play!=0 || jog!=0)
        for(ind=0;ind<52;ind++){
            n = ind/13;
            v = ind%13;
            if(carta_existe(data.jogadas[jog],n,v)){
                imprime_carta_imagem(path,x,y,n,v);
                x +=20;
            }
        }
      }
      x=550;
    }
    for(x=350,y=250,jog=3;jog>0;jog-=2,x+=420){//Pôr a ficar o jogador a somar?
      if (data.jogadas[jog]==0 && data.inicio!=1)
        printf("<image x = \"%d\" y = \"%d\" height = \"100\" width = \"100\" xlink:href = \"%s/passo_%d.svg\" />\n", x, y, path, jog);
      else
        for(ind=0;ind<52;ind++){
            n = ind/13;
            v = ind%13;
            if(carta_existe(data.jogadas[jog],n,v)){
                imprime_carta_imagem(path,x,y,n,v);
                y +=20;
            }
        }
      y=250;
    }
}




void imprime_maos (DATABASE data, char * path){
    int n,v,b,i;
    int x,y;
    int jog;
    int ind;
    if (data.baralhar) b = 13;
    else b = 1;
    for(y = 500, jog = 0; jog < 3; jog+=2, y -= 415)
        for(x = 445, ind = 0, i = 0; ind < 52; ind += b){
            n = ind/13;
            v = ind%13;
            if(jog==0){
                if(carta_existe(data.selected,n,v)==1)//desvia as cartas selecionadas
                    y -= 20;
                if(carta_existe(data.mao[jog],n,v)==1){
                    imprime_carta_link(path,x,y,data,n,v);
                    x += 20;
                }
                if(carta_existe(data.selected,n,v)==1)//anula o desvio, para n ficarem todas levantadas
                    y += 20;
            }
            else{
                n = ind/13;
                v = ind%13;
                if(carta_existe(data.mao[jog],n,v)==1){
                    imprime_carta_back(path,x,y);
                    x += 20;
                }
            }
            if (data.baralhar && ind > 38 && ind != 51) {
              i+=1;
              ind = i-13;
            }
        }
    for(x = 240, jog = 3; jog > 0; jog-=2, x += 640)//alterar para ser a somar o jog?
        for(y = 170, ind = 0, i = 0; ind < 52; ind += b){
            n = ind/13;
            v = ind%13;
            if(carta_existe(data.mao[jog],n,v)==1){
                imprime_carta_back(path,x,y);
                y += 20;
            }
            if (data.baralhar && ind > 38 && ind != 51) {
              i+=1;
              ind = i-13;
            }
        }

}



//##############################Funções para imprimir-FIM##############################################


//##############################Funções bots#######################################################


DATABASE bot_continua(DATABASE data,int m){
    int max;
    int ind;
    int n,v;
    max = maior_carta_jogada(data);
    data.jogadas[m] = 0;
    for(ind=0;ind<52;ind++){
        n=ind/13;
        v=ind%13;
        if(carta_existe(data.mao[m],n,v)==1)
            if(v>max%13||(v==max%13&&n>max/13)){
                data = check_cartas(n,v,data,m);
                break;
            }
    }
    if(data.jogadas[m]==0)
        data.passar++;
    else
        data.passar=0;
    if(data.mao[m]==0)
        data.play = 2;
    return data;
}
DATABASE bot_comeca(DATABASE data,int m){
    int jog;
    int ind;
    int n,v;
    for(jog=0;jog<4;jog++)
        data.jogadas[jog]=0;
    data.passar=0;
    for(ind=0;ind<52;ind++){
        n=ind/13;
        v=ind%13;
        if(carta_existe(data.mao[m],n,v)){
            data.jogadas[m]=add_carta(data.jogadas[m],n,v);
            data.mao[m] = rem_carta(data.mao[m],n,v);
            break;
        }
    }
    data.nc = 1;
    if(data.mao[m]==0)
        data.play = 2;
    return data;
}

DATABASE joga_bots(DATABASE data,int m){
    int i;
    if(all_passed(data)==1||data.nc==0){
        data = bot_comeca(data,m);
    }
    else{
        i = data.nc;
        data = bot_continua(data,m);
        data.nc = i;
    }
    return data;
}







//#################################Funções bots - FIM####################################

//######################################Função central#######################################



void imprime(char * path,DATABASE data){
    int jog;
      printf("<svg height = \"680\" width = \"1200\">\n");
    printf("<image x = \"-155\" y = \"0\" height = \"900\" width = \"1500\" xlink:href = \"%s/floor.svg\" />\n", path);
    printf("<circle cx=\"450\" cy=\"350\" r=\"290\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<circle cx=\"750\" cy=\"350\" r=\"290\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<rect x = \"450\" y = \"60\" height = \"580\" width = \"300\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<rect x = \"440\" y = \"70\" height = \"560\" width = \"320\" style = \"fill:#007700\"/>\n");
    if(data.mao[0]==0)
        imprime_fim(path,data);
    if(data.inicio == 1){
        jog = quem_comeca(data);
        if (jog>0)
            for(;jog<4;jog++){
                data=joga_bots(data,jog);
          if (jog == 3) data.play = 0;
            }
        data.inicio = 2;
    }
    if(data.play == 1)
        for(jog=1;jog<4 && data.play != 2;jog++){
            data = joga_bots(data,jog);
            if(data.play == 2)
                imprime_fim(path, data);
        }
    if(data.play != 2){
        imprime_jogadas(data,path);
        imprime_maos(data,path);
        imprime_play(data,path);
        imprime_passar(data,path);
        imprime_baralhar(data,path);
        imprime_help(data,path);
    }
}

//*****************************Função central- FIM################################



void check_start(char * path, DATABASE data){
    int y, x,p,ind,n,v;
    printf("<svg height = \"680\" width = \"1200\">\n");
    printf("<image x = \"-155\" y = \"0\" height = \"900\" width = \"1500\" xlink:href = \"%s/floor.svg\" />\n", path);
    printf("<circle cx=\"450\" cy=\"350\" r=\"290\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<circle cx=\"750\" cy=\"350\" r=\"290\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<rect x = \"450\" y = \"60\" height = \"580\" width = \"300\" stroke=\"maroon\" stroke-width=\"20\" style = \"fill:#007700\"/>\n");
    printf("<rect x = \"440\" y = \"70\" height = \"560\" width = \"320\" style = \"fill:#007700\"/>\n");
    if(data.inicio==0){
        for(y = 500, p = 0; p < 3; p+=2, y -= 415)
            for(x = 445, ind = 0; ind < 13; ind++){
                  imprime_carta_back(path,x,y);
                  x += 20;
            }
        for(x = 240, p = 3; p > 0; p-=2, x += 640)
            for(y = 170, ind = 0; ind < 13; ind++){
                  imprime_carta_back(path,x,y);
                  y += 20;
            }
        data.inicio=1;
        imprime_start(data,path);
        printf("</svg>\n");
    }
    else
        imprime(path,data);
        printf("</svg>\n");

}

void parse (char * query) {
    DATABASE data = {{0},0,{0},0,0,0,0,0,{0},{0}};
    if(query!=NULL && strlen(query) != 0){ //n sei para q é preciso a primeira condição...
        data = STR2DATA(query);
        if (data.mao[0]==0 && data.mao[1]==0 && data.mao[2]==0 && data.mao[3]==0) {
          data = distribui(data);
          check_start(BARALHO, data);
        }
        else imprime(BARALHO,data);
    }
    else{
        data = distribui(data);
        check_start(BARALHO, data);
    }
}


int main() {
    /*
     * Cabeçalhos necessários numa CGI
     */
    printf("Content-Type: text/html; charset=utf-8\n\n");
    printf("<header><title>Big Two</title></header>\n");
    printf("<body>\n");

    /*
     * Ler os valores passados à cgi que estão na variável ambiente e passá-los ao programa
     */
    srand(time(NULL));
    parse(getenv("QUERY_STRING"));
    printf("</body>\n");
    return 0;
}