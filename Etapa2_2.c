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

#define DATA "%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_%d"

typedef long long int MAO;
struct database{
    MAO mao[4];
    MAO selected;
    MAO jogadas[4];
    int play;
    int nc; //número de cartas da ronda
    int passar;
    int ordenar; //antigo baralhar.
    int score[4]; 
    int combination[3];
};

typedef struct database DATABASE;

/*
                                                    ##################################--AUXILIARES--#####################################
*/
//TODO: alterar de acordo com as alterações feitas à estrutura
DATABASE STR2DATA(char * str){
    DATABASE data;
    sscanf(str, DATA,&(data.mao[0]),&(data.mao[1]),&(data.mao[2]),&(data.mao[3]),&(data.selected),&(data.jogadas[0]),&(data.jogadas[1]),&(data.jogadas[2]),&(data.jogadas[3]),&data.play,&data.nc,&data.passar,&data.ordenar,&data.score[0],&data.score[1],&data.score[2],&data.score[3],&data.combination[0],&data.combination[1],&data.combination[2]);
    return data;
}

void DATA2STR(char * str,DATABASE data){
    sprintf(str,DATA,data.mao[0],data.mao[1],data.mao[2],data.mao[3],data.selected,data.jogadas[0],data.jogadas[1],data.jogadas[2],data.jogadas[3],data.play,data.nc,data.passar,data.ordenar,data.score[0],data.score[1],data.score[2],data.score[3],data.combination[0],data.combination[1],data.combination[2]);
}


/** \brief Adiciona uma carta a um dos estados
 
 @param ESTADO	O estado atual
 @param naipe	O naipe da carta (inteiro entre 0 e 3)
 @param valor	O valor da carta (inteiro entre 0 e 12)
 @return		O novo estado
 */
long long int add_carta(MAO ESTADO, int naipe, int valor) {
    int idx = naipe * 13 + valor;
    return ESTADO | ((long long int) 1 << idx);
}

/** \brief Remove uma carta a um dos estados
 
 @param ESTADO	O estado atual
 @param naipe	O naipe da carta (inteiro entre 0 e 3)
 @param valor	O valor da carta (inteiro entre 0 e 12)
 @return		O novo estado
 */
long long int rem_carta(MAO ESTADO, int naipe, int valor) {
    int idx = naipe * 13 + valor;
    return ESTADO & ~((long long int) 1 << idx);
}

/** \brief Verifica se uma carta pertence a um dos estados
 
 @param ESTADO	O estado atual
 @param naipe	O naipe da carta (inteiro entre 0 e 3)
 @param valor	O valor da carta (inteiro entre 0 e 12)
 @return		1 se a carta existe e 0 caso contrário
 */
int carta_existe(MAO ESTADO, int naipe, int valor) {
    int idx = naipe * 13 + valor;
    return (ESTADO >> idx) & 1;
}

void distribui(DATABASE * data){
    int maoCount[4]={0};
    int n,v,j;
    for(n=0;n<4;n++){
        v = 0;
        while(v<13){
            j = rand()%4;
            if (maoCount[j]<13){
                data->mao[j] = add_carta(data->mao[j],n,v);
                maoCount[j]++;
                v++;
            }
        }
    }
}

int calcula_score(MAO mao){
    int ind;
    int n,v;
    int r=0;
    
    for (ind=0;ind<52;ind++){
        n = ind/13;
        v = ind%13;
        if(carta_existe(mao,n,v))
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

//TODO: melhorar as duas abaixo

int maior_carta_mao(MAO mao){
    int ind;
    int max=-1;
    int n,v;
    
    for(ind=0;ind<52;ind++){
        n=ind/13;
        v=ind%13;
        if(carta_existe(mao,n,v)){
            if((v>max%13||(v==max%13&&n>max/13)) || max == -1)
                max = ind;
        }
    }
    return max;
}

int maior_carta_jogada(DATABASE * data){
    int jog;
    int n,v;
    int temp;
    int max = -1;
    
    for(jog=0;jog<4;jog++){
        temp = maior_carta_mao(data->jogadas[jog]);
        n = temp/13;
        v = temp%13;
        if((v>max%13||(v==max%13&&n>max/13)) || max == -1)
            max = temp;
    }
    return max;
}

int quem_comeca(DATABASE * data){
    int jog;
    for(jog=0;;jog++)
        if(carta_existe(data->mao[jog],0,0))
            break;
    return jog;
}

//WARNING: se metermos um count que faz a função parar aos 5 fica mais eficiente, não percorre as cartas todas
void separa_val (MAO ESTADO, int y[13]){
    int i,n,v;
    for(i=0;i<52;i++){
        n = i/13;
        v = i%13;
        if(carta_existe(ESTADO,n,v)){
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
        if(carta_existe(ESTADO,n,v)){
            y[n]++;
        }
    }
}


//acho que vai ser util criar um array de 6 na estrutura ou assim para as jogadas de 5 que diz os indices das cartas jogadas e na a[5] metemos o rank da jogada (se é straight flush e assim)
//se o em baixo der problemas, tentar este
 /*int teste_straight(int v[13]){
    int r = 0, i = 0, count = 0;
    for(i = 0; v[i] != 0; i++);
    for(;v[i] == 0; i = (i + 1) % 13);
    for(;v[i] != 0; i = (i + 1) % 13){
        count++;
    
    }
    if(count == 5)
        r = 1;
    return r;
}
  */

int teste_straight(int sa[13]){
    int r = 0, n = 0, c = 0;
    int ca[14];
    while (n < 12) {
        ca[n+2] = sa[n];
        n++;
    }
    ca[0] = sa[11];
    ca[1] = sa[12];
    for (n=0; n<15; n++) {
        if((ca[n])==1) c+=1;
        else c = 0;
        if (c==5) {
            n=15;
            r++;
        }
    }
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
//O 3 de ouros tem que ser jogado na primeira jogada, está nas regras(?)
//r: 1 => straight; 2 => flush; 3 => fullhouse; 4 => fourofakind; 5 => straightflush
int tipo_comb_five(MAO mao) {
    int r = 0;
    int n[4]={0};
    int v[13] = {0};
    separa_nap(mao, n);
    separa_val(mao, v);
    if(teste_straight(v) && teste_flush(n)) r = 5;
    if(r == 0 && teste_straight(v)) r = 1;
    if(r==0 && teste_flush(n)) r = 2;
    if(r==0 && teste_fullhouse(v)) r = 3;
    if(r==0 && teste_fourofakind(v)) r = 4;
    return r;
} 

void atualizastraight(MAO mao, int y[3]) {
  MAO m = mao;
  int ind, n, v;
    for (n=0, v=11; n!=4; n++)
      ind = v+1;
      if ((carta_existe(m, n, v)) && (carta_existe(m, n, ind))) {
        rem_carta(m, n, v);
        rem_carta(m, n, ind);
        n = 4;
    }
   ind = maior_carta_mao(m);
   y[1] = ind%13;
   y[2] = ind/13;
}

void preenchejogada (MAO mao, int y[3]){
  int m, i,ind;
  int rank[13] = {0};
  
  m = tipo_comb_five(mao);
  y[0] = m;
  printf("tipo:%d", y[0]);
  if (m==1 || m==5) atualizastraight(mao, y);
  else {
    if (m==2) {
      ind = maior_carta_mao(m);
      y[1] = ind%13;
      y[2] = ind/13;
    }
    else {
      separa_val(mao, rank);
      for(i=0; i<13; i++) {
        if (rank[i] >=3) break;
      }
      y[1] = i;
    }
  }
}

int cmpplay (MAO mao, int y[3]){
  int r=0;
  int t[3];
  if(mao != 0){
      	preenchejogada (mao,t);
    	if (t[0] > y[0])
            r=1;
        else
            if (t[0] == y[0]){
                if ((t[0] == 2) && ((t[2] > y[2]) || (t[2] == y[2] && t[1] > y[1])))
                    r = 1;
                if ((t[0] == 3 || t[0] == 4) && (t[1] > y[1]))
                    r = 1;
                if ((t[0] == 5) && (t[1] > y[1] || (t[1]==y[1] && t[2] > y[2])))
                    r = 1;
          }
  }
  return r;
}

long long int straightpos (MAO mao) {
  int i, c=0, n=0;
  MAO max = 0;
  int rank[13] = {0};
  separa_val(mao, rank);
  if(teste_straight(rank)) {
    for(i=11; i<13; i--) {
      if(rank[i]>0) c+=1;
      else c = 0;
      if(c==5) break;
      if(i==0) i=13;
    }
    c = 0;
    while (c<5) {
      if(carta_existe(mao, n, i)) {
        max = add_carta(max, n, i);
        c+=1;
        n=0;
        if(i==0) i=12;
        else i--;
      }
      else n++;
    }
  }
  else max = 0;
  return max;
}

long long int flushpos (MAO mao) {
  MAO max = 0;
  int i;
  int naipe[4] = {0};
  separa_nap(mao, naipe);
  for (i=4; i>=0; i--) {
    if(naipe[i]>4) break;
  }
  if(i!=4) {
    int v = 12;
    int c = 0;
    while(c<5) {
      if(carta_existe(mao, i, v)) {
        max = add_carta(max, i, v);
        c+=1;
      }
      v--;
    }
  }
  return max;
}

long long int fullhousepos (MAO mao) {
  MAO max = 0;
  int rank[13] = {0};
  separa_val(mao, rank);
  int i, p=0;
  for (i=13; i>=0; i--)
    if (rank[i]>=3) break;
  if(i==0) p=1;
  for (; p<13; p++) {
    if (rank[p]>=2) break;
    if ((p+1)==i) p = p+1;
  }
  if(i!=-1 && p!=13) {
    int n=0, c=0;
    while(c<3) {
      if(carta_existe(mao, n, i)) {
        max = add_carta(max, n, i);
        n++;
        c++;
      }
      else n++;
    }
    n=0;
    c=0;
    while(c<2) {
      if(carta_existe(mao, n, p)) {
        max = add_carta(max, n ,p);
        n++;
        c++;
      }
      else n++;
    }
  }
  return max;
}

long long int fourofakindpos (MAO mao) {
  MAO max = 0;
  int i;
  int rank[13] = {0};
  separa_val(mao, rank);
  for(i=13; i>=0; i--) 
    if(rank[i]==4) break;
  if(i!=-1) {
    int c=0, n=0, p;
    while(n<4) {
      max = add_carta(max, n, i);
      n++;
    }
    for(n=0; n<52; n++) {
      c = n/13;
      p = n%13;
      if(carta_existe(mao, c, p) && p!=i) {
        max = add_carta(max, c, p);
        break;
      }
    }
    if(n==52) max = 0;
  }
  return max;
}
//provavelmente temos de corrigir esta...
long long int straightflushpos (MAO mao) {
  MAO max = 0, temp = 0;
  int n=3;
  int naipe[4] = {0};
  separa_nap(mao, naipe);
  while(n>=0) {
    if(naipe[n]>4) {
      int v;
      for(v=13; v>=0; v--) {
        if(carta_existe(mao, n, v)) temp = add_carta(temp, n, v);
      }
      temp = straightpos(temp);
      if(temp==0) n--;
      else {
        max = temp;
        n = -1;
      }
    }
    else n--;
  }
  return max;
}

int joga5 (DATABASE * data, int m) {
    int r = 1;
    if (cmpplay(straightflushpos(data->mao[m]),data->combination)) data->jogadas[m] = straightflushpos(data->mao[m]);
    	else if(cmpplay(fourofakindpos(data->mao[m]),data->combination)) data->jogadas[m] = fourofakindpos(data->mao[m]);
    		 else if(cmpplay(fullhousepos(data->mao[m]),data->combination)) data->jogadas[m] = fullhousepos(data->mao[m]);
    			   else if(cmpplay(flushpos(data->mao[m]),data->combination)) data->jogadas[m] = flushpos(data->mao[m]);
                        else if(cmpplay(straightpos(data->mao[m]), data->combination)) data->jogadas[m] = straightpos(data->mao[m]);
                        else {
                            data->jogadas[m] = 0;
                            r = 0;
                        } //Fail safe
    return r;
}

int check_basico(DATABASE * data, int cartas[]){
    int max = maior_carta_jogada(data);
    int i, r = 0;
    int n, v;
    for(i = 0; i < data->nc; i++){
        n = cartas[i] / 13;
        v = cartas[i] % 13;
        if(v >= max % 13 && v == cartas[0] % 13){
            if(n > max / 13 || v > max % 13)
            	r = 1;
        }
        else{
            r = 0;
        	break;
        }
    }
    return r;
}


int check_jogada(DATABASE *data, int jog){
    int ind, r = 0, count = 0;
    int n, v;
    
    MAO jogada;
    int cartas[5];
    if(jog != 0)
        jogada = data->jogadas[jog];
    else
        jogada = data->selected;
    //Quando metermos as cinco, podemos dividir em duas funções, com a condição data.nc == 5, check_combinacões e check_basico ou algo do género
    //Pesquisar: existem algoritmos que contam números de bits a 1, num conjunto de bits, para saber o número de cartas numa mao.
    for(ind = 0; ind < 52 && count < 5; ind++){
        n = ind / 13;
        v = ind % 13;
        if (carta_existe(jogada,n,v)){
            cartas[count] = ind;
            count++; //incorporar este incremento no indice da expressão imediatamente acima;
        }
    }
    if(data->nc == count && count < 6 && count != 4){
        if(count < 5)
        	r = check_basico(data,cartas);
    	else
            if (data->passar==3 && tipo_comb_five(jogada) >0) r=1;
            else if(tipo_comb_five(jogada)>0  && cmpplay(jogada, data->combination)==1) r=1;
    }
    else
        r = 0;
    return r;
}

int determina_basico(DATABASE *data, int jog, int jogadas[][5]){
    int n, v, i, temp_naipe[4], count = 0, a;
    int max = maior_carta_jogada(data);
    for(v = max % 13; v < 13; v++){
        i = 0;
        for(n = 0; n < 4; n++)
            if(carta_existe(data->mao[jog],n,v)){
                temp_naipe[i] =n;
                i++; //meter este incremento em cima
            }
        if(i >= data->nc && (v > max % 13 || temp_naipe[i - 1] > max / 13)){
            for(a = 0, i = i -1; a < data->nc; i--, a++)
                jogadas[count][a] = temp_naipe[i] * 13 + v;
            count++;
        }
    }
    return count;
}

//TODO: separar em duas funções, uma para se data->nc = 5 e outra para as restantes, que irá ser igual a esta
//TODO: não testa direito entre pares com valores iguais, os naipes se são mais altos ou não
int jogadas_possiveis(DATABASE *data, int jog, int jogadas[][5]){
    int count = 0;
    if(data->nc != 5)
        count = determina_basico(data,jog,jogadas);
    //else
	//  ...  //WARNING: fazer dar logo zero se o data->nc for 4, depois quando metermos para as 5
    return count;
}

/* 
                                                ##################################--AUXILIARES_FIM--#####################################
 */
/*
                                                ##############################--FUNCOES_PARA_IMPRIMIR--##################################
 */

void imprime_carta_imagem(int x, int y,int n,int v){
    char *suit = NAIPES;
    char *rank = VALORES;
    printf("<image x = \"%d\" y = \"%d\" height = \"95\" width = \"70\" xlink:href = \"%s/%c%c.svg\" />\n", x, y, BARALHO, rank[v], suit[n]);
}

void imprime_carta_back(int x, int y){
    printf("<image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/card_back.svg\" />\n", x, y, BARALHO);
}

void imprime_carta_link(int x, int y,DATABASE data,int n,int v){
    char *suit = NAIPES;
    char *rank = VALORES;
    char script[52000]; // n sei quanto precisamos.
    if(carta_existe(data.selected,n,v)==0)
        data.selected = add_carta(data.selected,n,v);
    else
        data.selected = rem_carta(data.selected,n,v);
    data.play = 0;
    
    DATA2STR(script,data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" /></a>\n", script, x, y, BARALHO, rank[v], suit[n]);
}

//WARNING: não imprime os passo na primeira jogada
void imprime_jogadas(DATABASE data){
    int x,y;
    int n,v;
    int jog;
    int ind;
    
    for(x=550,y=210,jog=2;jog>=0;jog-=2,y+=175){
        if (data.jogadas[jog]==0 && data.play != 3) {
            if (data.play || jog!=0) {
                y -= 20;
                printf("<image x = \"%d\" y = \"%d\" height = \"100\" width = \"100\" xlink:href = \"%s/passo_%d.svg\" />\n", x, y, BARALHO, jog);
                y += 20;
            }
        }
        else {
            if (data.play!=0 || jog!=0)
                for(ind=0;ind<52;ind++){
                    n = ind/13;
                    v = ind%13;
                    if(carta_existe(data.jogadas[jog],n,v)){
                        imprime_carta_imagem(x,y,n,v);
                        x +=20;
                    }
                }
        }
        x=550;
    }
    for(x=350,y=250,jog=3;jog>0;jog-=2,x+=420){
        if (data.jogadas[jog]==0 && data.play != 3) //WARNING: verificar se é suposto ser 3
            printf("<image x = \"%d\" y = \"%d\" height = \"100\" width = \"100\" xlink:href = \"%s/passo_%d.svg\" />\n", x, y, BARALHO, jog);
        else
            for(ind=0;ind<52;ind++){
                n = ind/13;
                v = ind%13;
                if(carta_existe(data.jogadas[jog],n,v)){
                    imprime_carta_imagem(x,y,n,v);
                    y +=20;
                }
            }
        y=250;
    }
}


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
                if(carta_existe(data.selected,n,v)==1)//anula o desvio, para n ficarem todas levantadas
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


void botao_ordenar (DATABASE data){
    char script [52000];
    if (data.ordenar==0) {
        data.ordenar = 1;
        data.play = 0;
        DATA2STR(script, data);
        printf("<a xlink:href = \"cartas?%s\"><image x = \"350\" y = \"510\" height = \"40\" width = \"40\" xlink:href = \"%s/baralhar_por_valor.svg\" /></a>\n", script, BARALHO);
    }
    else {
        data.ordenar = 0;
        data.play = 0;
        DATA2STR(script, data);
        printf("<a xlink:href = \"cartas?%s\"><image x = \"350\" y = \"510\" height = \"40\" width = \"40\" xlink:href = \"%s/baralhar_por_naipe.svg\" /></a>\n", script, BARALHO);
    }
}


//TODO: Fazer com que dê todas as jogadas possíveis de forma seguida, pode ser preciso alterar a estrutura
//WARNING: não alterar o data.nc
void botao_help(DATABASE * data){
    char script[52000];
    int jogadas[13][5];
    int original = data->nc;
    int total = 0;
    int n, v, draw, i;
    if(data->passar != 3)
    	jogadas_possiveis(data, 0, jogadas);
    else{
        data->nc = rand() % 4;//TODO: alterar para 6 quando for com 5 cartas
        while(total == 0 && data->nc > 0){
            total = jogadas_possiveis(data, 0, jogadas);
            data->nc--;
        }
    }
    data->selected = 0;
    if(total != 0){
        draw = rand() % total;
        for(i = 0; i <= data->nc; i++){
            n = jogadas[draw][i] / 13;
            v = jogadas[draw][i] % 13;
            data->selected = add_carta(data->selected,n,v);
            
        }
    }
    data->play = 0;
    data->nc = original;
    DATA2STR(script,* data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"370\" y = \"560\" height = \"40\" width = \"40\" xlink:href = \"%s/botao_help.svg\" /></a>\n", script, BARALHO);
}




//TODO: meter botao a cinzento quando tem que jogar -> if(data.passar == 3)
void botao_passar (DATABASE data){
    char script[52000];
    
    data.selected = 0;
    data.jogadas[0]=0;
    data.play = 2;
    data.passar++;
    DATA2STR(script, data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"775\" y = \"550\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_pass.svg\" /></a>\n", script, BARALHO);
}


//melhorar esta função
void botao_play (DATABASE data){
    int n,v;
    int ind;
    int y[3];
    char script [52000];
    data.jogadas[0]=0;
    if(data.passar == 3||data.nc==0){ //WARNING\. acho que já não precisamos da segunda conição. data.nc == 0 significa que estámos na primeira jogada e somos nós a começar
        data.nc = 0;
        for(ind=0;ind<52;ind++){
            n=ind/13;
            v=ind%13;
            if(carta_existe(data.selected,n,v))
                data.nc ++;
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
        if(data.mao[0] == 0)
            data.play = 4; //4 -> fim do jogo
        else
            data.play = 2; //2 -> jogo normal
        
        DATA2STR(script, data);
        printf("<a xlink:href = \"cartas?%s\"><image x = \"775\" y = \"510\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_play.svg\" /></a>\n", script, BARALHO);
    }
    else
        printf("<image x = \"775\" y = \"510\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_play_cinza.svg\" />\n", BARALHO);
}


void botao_continuar(DATABASE * data) {
    int i;
    char script[52000];
    DATABASE newdata = {{0},0,{0},0,0,0,0,{0},{0}}; //TODO:tentar subsituir esta expressão com um define
    
    for(i=0; i < 4; i++)
        newdata.score[i] = data->score[i];
    distribui(&newdata);
    newdata.play = 1;
    DATA2STR(script,newdata);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"380\" y = \"500\" height = \"60\" width = \"170\" xlink:href = \"%s/botao_continuar.svg\" /></a>\n", script, BARALHO);
}

void botao_novojogo() {
    printf("<a xlink:href = \"cartas\"><image x = \"630\" y = \"500\" height = \"60\" width = \"170\" xlink:href = \"%s/botao_novo_jogo.svg\" /></a>\n", BARALHO);
}

//TODO: podemos simplificar isto com um for..
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
    printf("<text x = \"400\" y = \"310\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">Jogador - %d</text>\n", data->score[0]);
    printf("<text x = \"400\" y = \"350\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">Jorge - %d</text>\n", data->score[1]);
    printf("<text x = \"400\" y = \"390\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">Luís - %d</text>\n", data->score[2]);
    printf("<text x = \"400\" y = \"430\" style=\"font-family:Arial; fill:#ffffff; stroke:#000000; font-size:40px;\">Diogo - %d</text>\n", data->score[3]);
    botao_continuar(data);
    botao_novojogo();
    printf("</svg>");
}

//data.play = 1 está na imprime_start
void botao_start(DATABASE data){
    char script[52000]; //TODO: mudar o tamanho deste script, é demasiado grande
    
    DATA2STR(script, data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"510\" y = \"300\" height = \"60\" width = \"180\" xlink:href = \"%s/botao_start.svg\" /></a>\n", script, BARALHO);
}

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
    botao_ordenar(data);//antigo imprime_baralhar
    botao_help(&data);
}


/*
                                                #############################--FUNCOES_PARA_IMPRIMIR-FIM--###############################
 */
/*
 												#################################--FUNCOES_DOS_BOTS--####################################
 */

//TODO: substituir o m por jog, para manter igual isto em todas as funções
void bot_continua(DATABASE *data,int m){
    int draw, total, i, n, v;
    int jogadas[15][5]; //Primeiro elemento do array => número da jogada_possível; o 15 em vez de 13 é só por segurança. Segundo elemento do array => carta da jogada_possível. Exemplo: jogadas[0][0] e jogadas[0][1] dão o par da jogada possível nr. 1.
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
    if(data->jogadas[m]!=0) data->passar = 0;
    else data->passar++;
    if(data->mao[m]==0)
        data->play = 4;
}


void bot_comeca(DATABASE *data,int m){
    int jog, total = 0, i; //total => número total de jogadas possíveis
    int jogadas[15][5];
    int draw;
    int n, v;
    int y[3];
    
    for(jog=0;jog<4;jog++)
        data->jogadas[jog]=0; //TODO: testar se é preciso, acho que nunca altera nada, basta data->jogadas[m] = 0 e depois substituir m por jog, para manter consistência nisto
    data->passar = 0;
    data->nc = 5; //para começar a testar combinações de 3 primeiro, depois mudar para 6 para testar combinações de 5 primeiro
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

    if(data->mao[m]==0)
        data->play = 4;
}


//WARNING: no if, tinha i = data.nc no inicio e data.nc = i no fim
void joga_bots(DATABASE *data,int m){
    if(data->passar != 3)
        bot_continua(data,m);
    else
        bot_comeca(data,m);
}


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



/* 
 											    ##################################--FUNCOES_DOS_BOTS_FIM--##############################
 */



//Adicionar imprime_fim aqui?
//Fazer passar o endereço da estrutura em vez de cópias?
void Game_Lobby(DATABASE data){
    switch (data.play) {
        case 0:
            imprime(data);
            break;
            
        case 1:
            imprime_start(data); //antigo check_start
            break;
            
        case 4:
            imprime_fim(&data);
            
        default: // data.play == 2 -> jogo normal. data.play == 3 -> inicio do jogo. quem se preocupa com este 3 é a função jogo. 4 -> fim do jogo
            jogo(&data);
            if(data.play != 4)
            	imprime(data);
            break; //WARNING: acho que se pode tirar este break
    }
}


void parse (char * query) {
    DATABASE data = {{0},0,{0},0,0,0,0,{0},{0}}; //vale a pena inicializar tudo a 0's sempre?
    
    if(query!=NULL && strlen(query) != 0) //meter condição adicional, para novo jogo e continuar, talvez data.play = 5 e depois já n é preciso aquela função enorme, basta mandar para baixo e manter os scores ou não, conforme o que se queira.
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

