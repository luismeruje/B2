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

/**
 Estrutura de dados:
 
 mao[4]            Mao de cada jogador
 selected          Cartas selecionadas
 jogadas[4]        Cartas da ultima jogada de cada jogador
 play              Identificador do tipo de jogo: 1 ->Começar;  2 ->Jogo normal; 3 ->Inicio do jogo; 4 ->Fim do jogo.
 nc;               Número de cartas da ronda
 passar;           Contagem de passos
 ordenar;          Identificador do tipo de ordenamento de cartas
 score[4];         Pontuação de cada jogador
 combination[3];   Posição 0: Tipo de combinação de 5 cartas. Posição 1: Valor da maior carta. Posição 2: Naipe da maior carta.
 */
struct database{
    MAO mao[4];
    MAO selected;
    MAO jogadas[4];
    int play;
    int nc;
    int passar;
    int ordenar;
    int score[4]; 
    int combination[3];
};

typedef struct database DATABASE;

/*
                                                    ##################################--AUXILIARES--#####################################
*/

/** \brief Converte a query para a estrutura de dados
 
 @param str     Query
 @return        Estrutura de dados
 */
DATABASE STR2DATA(char * str){
    DATABASE data;
    sscanf(str, DATA,&(data.mao[0]),&(data.mao[1]),&(data.mao[2]),&(data.mao[3]),&(data.selected),&(data.jogadas[0]),&(data.jogadas[1]),&(data.jogadas[2]),&(data.jogadas[3]),&data.play,&data.nc,&data.passar,&data.ordenar,&data.score[0],&data.score[1],&data.score[2],&data.score[3],&data.combination[0],&data.combination[1],&data.combination[2]);
    return data;
}

/** \brief Converte a estrutura de dados para query
 
 @param str     Estrutura de dados
 @return        Query
 */
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

/** \brief Distribui cartas pelas 4 mãos
 
 @param data    Estrutura atual
 */
void distribui(DATABASE * data){
    int maoCount[4] = {0};
    int n,v,j;
    for(n = 0; n < 4; n++){
        v = 0;
        while(v < 13){
            j = rand()%4;
            if (maoCount[j]<13){
                data->mao[j] = add_carta(data->mao[j],n,v);
                maoCount[j]++;
                v++;
            }
        }
    }
}

/** \brief Calcula a pontuação de uma MAO
 
 @param mao     A mão com as cartas do jogador
 @return        A pontuação
 */
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
    return -r;
}

/** \brief Calcula a maior carta de uma MAO
 
 @param mao     A mão com as cartas do jogador
 @return        O índice da maior carta
 */
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

/** \brief Calcula a maior carta jogada
 
 @param data    Estrutura atual
 @return        O índice da maior carta jogada
 */
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

/** \brief Calcula o primeiro jogador a começar
 
 @param data    Estrutura atual
 @return        O primeiro jogador a começar
 */
int quem_comeca(DATABASE * data){
    int jog;
    for(jog=0;;jog++)
        if(carta_existe(data->mao[jog],0,0))
            break;
    return jog;
}

/** \brief Transforma um array para que tenha o número de ocorrências de cada valor numa mão
 
 @param mao     A mão com as cartas do jogador
 @param rank    Array a ser modificado
 */
void separa_val (MAO mao, int rank[13]){
    int i,n,v;
    for(i=0;i<52;i++){
        n = i/13;
        v = i%13;
        if(carta_existe(mao,n,v)){
            rank[v]++;
        }
    }
}

/** \brief Transforma um array para que tenha o número de ocorrências de cada naipe numa mão
 
 @param mao     A mão com as cartas do jogador
 @param naipe   Array a ser modificado
 */
void separa_nap (MAO mao, int naipe[4]){
    int i,n,v;
    for(i=0;i<52;i++){
        n = i/13;
        v = i%13;
        if(carta_existe(mao,n,v)){
            naipe[n]++;
        }
    }
}

/** \brief Testa se uma mão de 5 cartas é um straiht
 
 @param v       Array com o número de ocorrências de cada valor
 @return        Um int com valor 1 se verdadeiro ou 0 se falso
 */
int teste_straight(int v[13]){
    int r = 0, i = 0, count = 0, flag = 0;
    
    for(i = 0; v[i] != 0; i++);
    for(;v[i] == 0; i = (i + 1) % 13);
    for(;v[i] != 0; i = (i + 1) % 13){
        count++;
    	if(i == 11 && count != 1 && count != 5) //testa se o Ás não é o último nem o primeiro da sequencia
            flag = 1;
    }
    if(count == 5 && flag == 0)
        r = 1;
    return r;
}

/** \brief Testa se uma mão de 5 cartas é um flush
 
 @param naipe   Array com o número de ocorrências de cada naipe
 @return        Um int com valor 1 se verdadeiro ou 0 se falso
 */
int teste_flush(int naipe[4]) {
    int r = 0, n;
    for (n=0; n<4; n++) {
        if(naipe[n]==5) {
            r = 1;
            break;
        }
    }
    return r;
}

/** \brief Testa se uma mão de 5 cartas é um full house
 
 @param v       Array com o número de ocorrências de cada valor
 @return        Um int com valor 1 se verdadeiro ou 0 se falso
 */
int teste_fullhouse(int rank[13]) {
    int r = 1, v;
    for (v=0; v<13; v++) {
        if (rank[v]==4 || rank[v]==1) r = 0;
    }
    return r;
}

/** \brief Testa se uma mão de 5 cartas é um four of a kind
 
 @param v       Array com o número de ocorrências de cada valor
 @return        Um int com valor 1 se verdadeiro ou 0 se falso
 */
int teste_fourofakind(int rank[13]) {
    int r = 0, v;
    for (v=0; v<13; v++) {
        if (rank[v]==4) r = 1;
    }
    return r;
}

/** \brief Devolve o tipo de combinação de uma mão de 5 cartas
 
 @param mao     A mão com as cartas
 @return        Um int com valor: 0 caso não seja nenhuma combinação; 1 caso straight; 2 caso flush; 3 caso full house; 4 caso four of a kind; 5 caso straight flush.
 */
int tipo_comb_five(MAO mao) {
    int r = 0;
    int n[4]={0};
    int v[13] = {0};
    separa_nap(mao, n);
    separa_val(mao, v);
    if(teste_straight(v)){
    	if(teste_flush(n))
            r = 5;
    	else
            r = 1;
    }
    else if(teste_flush(n)) r = 2;
    	 else if(teste_fullhouse(v)) r = 3;
              else if(teste_fourofakind(v)) r = 4;
    return r;
} 

/** \brief Atualiza o array da combination consoante o straight da mão
 
 @param mao     A mão com as cartas
 @param y       Array correspondente ao combination
 */
//Soma-se um aos v's de cada carta, assim se o ás for última carta tem o maior v. Tbm conta com se o ás for a primeira carta
void atualizastraight(MAO mao, int y[3]) {
    int ind = 0, i;
    int v[13] = {0};
    separa_val(mao, v);
    if (v[11] != 0 && v[12] != 0){ //temos um ás e é o primeiro da sequência, pq existe um 2
        y[1] = 3; //5 de alguma coisa é obrigatoriamente a carta mais alta
        for(i = 0; i < 4; i++)
            if(carta_existe(mao, i, 2)){ //Determina o naipe do 5
                y[2] = i;
                break;
            }
    }
	else {
        for(i = 0; i < 4; i++)
            if(carta_existe(mao,i, 12))
            	mao = rem_carta(mao, i, 12); //remove os 2's
        ind = maior_carta_mao(mao);
        y[1] = (ind % 13) + 1;
        y[2] = ind / 13;
    }
}

/** \brief Atualiza o array da combination consoante a combinação da mão
 
 @param mao     A mão com as cartas
 @param y       Array correspondente ao combination
 */
void preenchejogada (MAO mao, int y[3]){
  int m, i,ind;
  int rank[13] = {0};
  m = tipo_comb_five(mao);
  y[0] = m;
  if (m==1 || m==5) atualizastraight(mao, y);
  else {
    if (m==2) {
      ind = maior_carta_mao(mao);
      y[1] = ind%13;
      y[2] = ind/13;
    }
    else {
      separa_val(mao, rank);
      for(i=0; i<13; i++) {
        if (rank[i] >=3) break;
      }
      y[1] = i;
        y[2] = 0;
    }
  }
}

/** \brief Compara a combinação de 5 cartas da mão com o array combination da estrutura
 
 @param mao     A mão com as cartas
 @param y       Array correspondente ao combination
 @return        1 caso seja maior a combinação ou 0 caso não seja
 */
int cmpplay (MAO mao, int y[3]){
  int r=0;
  int t[3];
  if(mao != 0){
      	preenchejogada (mao,t);
    	if (t[0] > y[0])
            r=1;
        else
            if (t[0] == y[0]){
                if ((t[0]==1) && ((t[1] > y[1]) || (t[1] == y[1] && t[2] >= y[2])))
                    r = 1;
                if ((t[0] == 2) && ((t[2] > y[2]) || (t[2] == y[2] && t[1] >= y[1])))
                    r = 1;
                if ((t[0] == 3 || t[0] == 4) && (t[1] >= y[1]))
                    r = 1;
                if ((t[0] == 5) && (t[1] > y[1] || (t[1]==y[1] && t[2] >= y[2])))
                    r = 1;
          }
  }
  return r;
}

/** \brief Calcula uma combinação straight apartir de uma mão
 
 @param mao     A mão com as cartas
 @return        Uma mão com a combinação straight possível
 */
MAO straightpos (MAO mao) {
    int c=0, n, v;
  	MAO max = 0;
  	int rank[13] = {0};
  	separa_val(mao, rank);
    for(v = 11 ; v >= 0 ; v--){
     	if(rank[v] != 0)
            c++;
		else
            c = 0;
        if (c == 5)
            break;
    }
    if(c == 5 || (c == 4 && rank[12] != 0) || (c == 3 && rank[12] != 0 && rank[11] != 0)) {
        if((c == 4 && rank[12] != 0))
            v = 12;
    	if (c == 3 && rank[12] != 0 && rank[11] != 0)
            v = 11;
        for(c = 0; c < 5; c++, v = (v + 1) % 13)
            for(n = 0; n < 4; n++)
                if(carta_existe(mao, n, v)){
                    max = add_carta(max, n, v);
                    n = 4;
                }
    }
  	return max;
}

/** \brief Calcula uma combinação flush apartir de uma mão
 
 @param mao     A mão com as cartas
 @return        Uma mão com a combinação flush possível
 */
MAO flushpos (MAO mao) {
  MAO max = 0;
  int i;
  int naipe[4] = {0};
  separa_nap(mao, naipe);
  for (i=3; i>=0; i--) if(naipe[i]>4) break;
  if(i != -1) {
    int v = 12, c = 0;
    while (c < 5) {
    	if(carta_existe(mao, i, v)) {
        	max = add_carta(max, i, v);
        	c+=1;
      	}
      	v--;
    }
  }
  return max;
}

/** \brief Calcula uma combinação full house apartir de uma mão
 
 @param mao     A mão com as cartas
 @return        Uma mão com a combinação full house possível
 */
MAO fullhousepos (MAO mao) {
  MAO max = 0;
  int rank[13] = {0};
  int i, p = 0;
  separa_val(mao, rank);
  for (i=12; i>=0; i--) if (rank[i]>=3) break;
  if (i == 0) p = 1;
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
    while(c < 2) {
      if(carta_existe(mao, n, p)) {
        max = add_carta(max, n ,p);
        c++;
      }
      n++;
    }
  }
  return max;
}

/** \brief Calcula uma combinação four of a kind apartir de uma mão
 
 @param mao     A mão com as cartas
 @return        Uma mão com a combinação four of a kind possível
 */
MAO fourofakindpos (MAO mao) {
  MAO max = 0;
  int i;
  int rank[13] = {0};
  separa_val(mao, rank);
  for(i=12; i>=0; i--) if(rank[i]==4) break;
  if(i!=-1) {
    int c=0, n=0, p;
    while(n < 4) {
      max = add_carta(max, n, i);
      n++;
    }
    for(n=0; n<52; n++) {
      c = n / 13;
      p = n % 13;
      if(carta_existe(mao, c, p) && p != i) {
        max = add_carta(max, c, p);
        break;
      }
    }
    if(n==52) max = 0;
  }
  return max;
}

/** \brief Calcula uma combinação straight flush apartir de uma mão
 
 @param mao     A mão com as cartas
 @return        Uma mão com a combinação straight flush possível
 */
MAO straightflushpos (MAO mao) {
  MAO max = 0, temp = 0;
  int n=3;
  int naipe[4] = {0};
  separa_nap(mao, naipe);
  while(n>=0) {
    if(naipe[n]>4) {
      int v;
      for(v=12; v>=0; v--) {
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

/** \brief Calcula uma jogada de 5 cartas da mão do jogador m
 
 @param data    Estrutura atual
 @param m       Jogador a jogar
 @return        Devolve: 1 caso encontre jogada; 0 caso não encontre
 */
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
                             }
    return r;
}

/** \brief Calcula uma jogada four of a kind com o 3 de ouros da mão do jogador m
 
 @param data    Estrutura atual
 @param m       Jogador a jogar
 */
void pmjgfourofakind (DATABASE * data, int m) {
    int ind, n, v;
    int rank[13] = {0};
    separa_val(data->mao[m],rank);
    MAO temp = 0;
    if(rank[0]==4) {
        for(n = 0; n < 4; n++)
            if(carta_existe(data->mao[m],n,0))
                temp = add_carta(temp,n,0);
        for(ind = 0; ind < 52; ind++){
            n = ind / 13;
            v = ind % 13;
            if(carta_existe(data->mao[m],n,v) && v != 0){
                temp = add_carta(temp,n,v);
                break;
            }
        }
    }
    else {
        for(v=1; v<13; v++) if(rank[v]==4) break;
        if (v != 13) {
            for(n = 0; n < 4; n++)
                if(carta_existe(data->mao[m],n,v))
                    temp = add_carta(temp,n,v);
            temp = add_carta(temp, 0, 0);
        }
    }
    data->jogadas[m] = fourofakindpos(temp);
}

/** \brief Calcula uma jogada full house com o 3 de ouros da mão do jogador m
 
 @param data    Estrutura atual
 @param m       Jogador a jogar
 */
void pmjgfullhouse (DATABASE * data, int m) {
    int n, v, count = 0;
    int rank[13] = {0};
    separa_val(data->mao[m],rank);
    MAO temp = 0;
    if(rank[0]==3) {
        for(n = 0; n < 4; n++)
            if(carta_existe(data->mao[m],n,0))
                temp = add_carta(temp,n,0);
        for(v = 1; v < 13; v++)
            if(rank[v] >= 2){
                for(n = 0; n < 4 && count < 2; n++)
                    if(carta_existe(data->mao[m],n,v)){
                        temp = add_carta(temp,n,v);
                        count++ ;
                    }
                break;
            }
        if(count == 2) data->jogadas[m] = temp;
    }
    else if(rank[0]==2) {
        for(n = 0; n < 4; n++)
            if(carta_existe(data->mao[m],n,0)){
                temp = add_carta(temp,n,0);
            }
        for(v = 1; v < 13; v++)
            if(rank[v] >= 3){
                for(n = 0; n < 4 && count < 3; n++)
                    if(carta_existe(data->mao[m],n,v)){
                        temp = add_carta(temp,n,v);
                        count++ ;
                    }
                break;
            }
        if(count == 3) data->jogadas[m] = temp;
    }
}

/** \brief Calcula uma jogada flush com o 3 de ouros da mão do jogador m
 
 @param data    Estrutura atual
 @param m       Jogador a jogar
 */
void pmjgflush (DATABASE * data, int m) {
    int valor, count = 0;
    MAO temp = 0;
    for(valor = 0; valor < 13 && count < 5; valor++)
        if(carta_existe(data->mao[m],0, valor)){
            temp = add_carta(temp, 0, valor);
            count++;
        }
    if(count==5) data->jogadas[m] = temp;
}

/** \brief Calcula uma jogada com o 3 de ouros da mão do jogador m
 
 @param data    Estrutura atual
 @param m       Jogador a jogar
 */
void primeira_jogada(DATABASE * data, int m){
    long long int temp = data->mao[m];
    int ind, n, v, count = 0;
    int rank[13] = {0};
    separa_val(data->mao[m],rank);
    data->nc = 5;
    //eliminar todas as cartas que não podem estar num straight com 3 de ouros, incluindo outros 3, que não sejam o de ouros
    for(ind = 0; ind < 52; ind++){
        n = ind / 13;
        v = ind % 13;
        if(carta_existe(temp, n, v) && ((v > 4 && v < 11) || (v == 0 && n != 0)))
            temp = rem_carta(temp, n, v);
    }
    data->jogadas[m] = straightflushpos(temp);
    if(data->jogadas[m] == 0) data->jogadas[m] = straightpos(temp);
    if(data->jogadas[m] == 0) pmjgfourofakind(data, m);
    if(data->jogadas[m] == 0) pmjgfullhouse(data, m);
    if(data->jogadas[m] == 0) pmjgflush(data, m);
    if(data->jogadas[m] == 0){
        for(n=0, count=0; n<4; n++)
            if(carta_existe(data->mao[m], n, 0)) {
                data->jogadas[m] = add_carta(data->jogadas[m], n, 0);
                count++;
            }
        data->nc = count;
    }
}

/** \brief Testa se uma jogada entre 1 a 3 cartas é válida
 
 @param data    Estrutura atual
 @param cartas  Array com os indices das cartas
 @return        Devolve: 1 caso a jogada seja válida; 0 caso não seja
 */
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

/** \brief Testa se uma jogada do jogador jog é válida
 
 @param data    Estrutura atual
 @param jog     Jogador a jogar
 @return        Devolve: 1 caso a jogada seja válida; 0 caso não seja
 */
int check_jogada(DATABASE *data, int jog){
    int ind, r = 0, count = 0;
    int n, v;
    MAO jogada;
    int cartas[5];
    if(jog != 0)
        jogada = data->jogadas[jog];
    else
        jogada = data->selected;
    for(ind = 0; ind < 52 && count < 5; ind++){
        n = ind / 13;
        v = ind % 13;
        if (carta_existe(jogada,n,v)){
            cartas[count] = ind;
            count++;
        }
    }
    if(data->nc == count && count < 6 && count != 4){
        if(count < 5)
        	r = check_basico(data,cartas);
    	else
            if (data->passar==3 && tipo_comb_five(jogada) >0) r=1;
            else if(tipo_comb_five(jogada)>0  && cmpplay(jogada, data->combination)==1) r=1;
        if(data->play==3) {
            if(!(carta_existe(jogada, 0, 0))) r=0;
        }
    }
    else
        r = 0;
    return r;
}

/** \brief Testa se uma jogada do jogador jog é válida
 
 @param data    Estrutura atual
 @param jog     Jogador a jogar
 @return        Determina jogadas com entre 1 a 3 cartas
 */
int jogadas_possiveis(DATABASE *data, int jog, int jogadas[][5]){
    int n, v, i, temp_naipe[4], count = 0, a;
    int max = maior_carta_jogada(data);
    if (max==-1) v=0;
    else v=max%13;
    for(; v < 13; v++){
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

/* 
                                                ##################################--AUXILIARES_FIM--#####################################
 */
/*
                                                ##############################--FUNCOES_PARA_IMPRIMIR--##################################
 */

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


/*
                                                #############################--FUNCOES_PARA_IMPRIMIR-FIM--###############################
 */
/*
 												#################################--FUNCOES_DOS_BOTS--####################################
 */

/** \brief Bot a continuar uma jogada
 
 @param data    Estrutura atual
 @param m       Jogador a continuar
*/
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
    if(data->jogadas[m]!=0){
        data->passar = 0;
    }
    else {
        data->passar++;
    }
    if(data->mao[m]==0)
        data->play = 4;
}

/** \brief Bot a começar uma jogada
 
 @param data    Estrutura atual
 @param m       Jogador a começar
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

/** \brief Responsável Pela Jogada dos Bots
 
 @param data    Estrutura atual
 @param m       Jogador a continuar
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



/* 
 											    ##################################--FUNCOES_DOS_BOTS_FIM--##############################
 */

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

