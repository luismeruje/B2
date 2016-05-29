#include "estrutura.h"


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
 
 @param mao     Mão com as cartas do jogador
 @return        Pontuação
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
 
 @param mao     Mão que queremos analisar
 @return        Índice da maior carta
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

/** \brief Calcula a maior carta jogada, de entre as mãos do parâmetro "jogadas" da estrutura
 
 @param data    Estrutura atual
 @return        Índice da maior carta jogada
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

/** \brief Determina quem começa o jogo
 
 @param data    Estrutura atual
 @return        Jogador que vai começar o jogo
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

/** \brief Testa se uma mão de 5 cartas é um straight
 
 @param v       Array com o número de ocorrências de cada valor
 @return        Um int com valor 1 se for straight ou 0 se não for
 */
int teste_straight(int v[13]){
    int r = 0, i = 0, count = 0, flag = 0;
    
    for(i = 0; v[i] != 0; i++);
    for(;v[i] == 0; i = (i + 1) % 13);
    for(;v[i] != 0; i = (i + 1) % 13){
        count++;
    	if(i == 11 && count != 1 && count != 5) 
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

/** \brief Atualiza o array que vai preencher o parâmetro combinations consoante o straight da mão
 
 @param mao     A mão com as cartas
 @param y       Array correspondente ao combination
 */

void atualizastraight(MAO mao, int y[3]) {
    int ind = 0, i;
    int v[13] = {0};
    separa_val(mao, v);
    if (v[11] != 0 && v[12] != 0){ 
        y[1] = 3; 
        for(i = 0; i < 4; i++)
            if(carta_existe(mao, i, 2)){ 
                y[2] = i;
                break;
            }
    }
	else {
        for(i = 0; i < 4; i++)
            if(carta_existe(mao,i, 12))
            	mao = rem_carta(mao, i, 12); 
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

/** \brief Calcula uma combinação straight a partir de uma mão
 
 @param mao     Mão com as cartas
 @return        Mão com a combinação straight possível
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
 
 @param mao     Mão com as cartas
 @return        Mão com a combinação flush possível
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
 
 @param mao     Mão com as cartas
 @return        Mão com a combinação full house possível
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
 
 @param mao     Mão com as cartas
 @return        Mão com a combinação four of a kind possível
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
 
 @param mao     Mão com as cartas
 @return        Mão com a combinação straight flush possível
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
    long long int temp = 0;
    int ind, n, v;
    int rank[13] = {0};
    separa_val(data->mao[m],rank);
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
    long long int temp = 0;
    int n, v, count = 0;
    int rank[13] = {0};
    separa_val(data->mao[m],rank);
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
                i++;
            }
        if(i >= data->nc && (v > max % 13 || temp_naipe[i - 1] > max / 13)){
            for(a = 0, i = i -1; a < data->nc; i--, a++)
                jogadas[count][a] = temp_naipe[i] * 13 + v;
            count++;
        }
    }
    return count;
}
