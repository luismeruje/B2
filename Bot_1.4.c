#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define MAO_INICIAL "MAO %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c"
#define JOGADA "JOGOU %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c %c%c"

typedef long long int MAO;

typedef struct database{
    MAO mao[4];
    int nc;
    MAO jogada;
    int passar;
    MAO usadas[4];
    int firstplay;
} DATABASE;

typedef struct searchtree{
    MAO estado;
    long int t;
    float r;
    int play;
    struct searchtree * nextN[4][40];
    struct searchtree * prev;
    int selected;
} searchTree;

typedef searchTree* MCtree;





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

void separa_val (MAO mao, int y[13]){
    int i,n,v;
    for(i=0;i<52;i++){
        n = i/13;
        v = i%13;
        if(carta_existe(mao,n,v)){
            y[v]++;
        }
    }
}

void separa_nap (MAO mao, int y[4]){
    int i,n,v;
    for(i=0;i<52;i++){
        n = i/13;
        v = i%13;
        if(carta_existe(mao,n,v)){
            y[n]++;
        }
    }
}

int teste_straight(int v[13]){
    int r = 0, i = 0, count = 0, flag = 0;
    
    for(i = 0; v[i] != 0; i++);
    for(;v[i] == 0; i = (i + 1) % 13);
    for(;v[i] != 0; i = (i + 1) % 13){
        count++;
        if(i == 11 && count != 1 && count != 5) /*testa se o Ás não é o último nem o primeiro da sequencia */
            flag = 1;
    }
    if(count == 5 && flag == 0)
        r = 1;
    return r;
}

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

/* r: 1 => straight; 2 => flush; 3 => fullhouse; 4 => fourofakind; 5 => straightflush
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

void atualizastraight(int v[13], MAO mao, int y[2]) {
    int ind = 0, i;
    if (v[11] != 0 && v[12] != 0){ /*temos um ás e é o primeiro da sequência, pq existe um 2 */
        y[0] = 3; /*5 de alguma coisa é obrigatoriamente a carta mais alta*/
        for(i = 0; i < 4; i++)
            if(carta_existe(mao, i, 2)){
                y[1] = i;
                break;
            }
    }
    else {
        for(i = 0; i < 4; i++)
            if(carta_existe(mao,i, 12))
                mao = rem_carta(mao, i, 12); /*remove os 2's*/
        ind = maior_carta_mao(mao);
        y[0] = (ind % 13) + 1;
        y[1] = ind / 13;
    }
}


int straightpos (DATABASE * simulacao, int jog, int sametype, MAO jogadas[40], int count) {
    int c=0, n, v = 11, vn[2], i= 0,count2, maxi;
    int straights[15]={-1};
    int rank[13] = {0};
    int tempN[15] = {-1};
    separa_val(simulacao->mao[jog], rank);
    if(sametype != 0){
        atualizastraight(rank, simulacao->jogada, vn);
        v = vn[0] - 4;
        if(v < 0)
            v = 13 + v;
    }
    if(v >= 7 && v <11)
        v = 11;
    for(; v != 7; v = (v + 1) % 13){
        if(rank[v] > 0)
            c++;
    	else
            c = 0;
        if(c == 5){
            if(sametype != 0){
                if(v > vn[0]){
                    v = v - 4;
                    if(v < 0)
                        v = 13 + v;
                    i++;
                    c = 0;
                }
                else{
                    for(n = 3; n >= 0; n--)
                        if(carta_existe(simulacao->mao[jog],n,v))
                            break;
                    if(n > vn[1]){
                        tempN[i] = n;
                        v = v - 4;
                        if(v < 0)
                            v = 13 + v;
                        straights[i] = v;
                        i++;
                        c = 0;
                    }
                    else{
                        c = 0;
                    }
                }
        	}
            else{
                v = v - 4;
                if(v < 0)
                    v = 13 + v;
                straights[i] = v;
                i++;
                c = 0;
            }
        }
    }
    if(i > 0)
        for(count2 = 0; count2 < i ; count2++){
            jogadas[count] = 0;
            for(v = straights[count2], maxi = (straights[count2] + 4) % 13 ; v != (maxi + 1) % 13; v= (v+1) % 13){
                if (v != maxi){
                    for(n = 0; n<4;n++)
                        if(carta_existe(simulacao->mao[jog],n,v)){
                            jogadas[count] = add_carta(jogadas[count],n,v);
                            n = 4;
                        }
                }
                else{
                    if(sametype != 0){
                      	jogadas[count] = add_carta(jogadas[count], tempN[count2],v);
                    	count++;
                    }
                    else{
                        for(n = 0; n<4;n++)
                            if(carta_existe(simulacao->mao[jog],n,v)){
                                jogadas[count] = add_carta(jogadas[count],n,v);
                                n = 4;
                                count++;
                            }
                    }
                }
            }
        }
    return count;
}


int straightpos2 (MAO mao, DATABASE * simulacao, int jog, int sametype, MAO jogadas[40], int count) {
    int c=0, n, v = 11, vn[2], i= 0,count2, maxi;
    int straights[15]={-1};
    int rank[13] = {0};
    int tempN[15] = {-1};
    separa_val(mao, rank);
    if(sametype != 0){
        atualizastraight(rank, simulacao->jogada, vn);
        v = vn[0] - 4;
        if(v < 0)
            v = 13 + v;
    }
    for(; v != 7; v = (v + 1) % 13){
        if(rank[v] > 0)
            c++;
        else
            c = 0;
        if(c == 5){
            if(sametype != 0){
                if(v > vn[0]){
                    v = v - 4;
                    if(v < 0)
                        v = 13 + v;
                    straights[i] = v;
                    i++;
                    c = 0;
                }
                else{
                    for(n = 3; n >= 0; n--)
                        if(carta_existe(simulacao->mao[jog],n,v))
                            break;
                    if(n > vn[1]){
                        tempN[i] = n;
                        v = v - 4;
                        if(v < 0)
                            v = 13 + v;
                        straights[i] = v;
                        i++;
                        c = 0;
                    }
                    else{
                        c = 0;
                    }
                }
            }
            else{
                v = v - 4;
                if(v < 0)
                    v = 13 + v;
                straights[i] = v;
                i++;
                c = 0;
            }
        }
    }
    if(i > 0)
        for(count2 = 0; count2 < i ; count2++){
            jogadas[count] = 0;
            for(v = straights[count2], maxi = straights[count2] + 4 ; v <= maxi; v++){
                if (v != maxi){
                    for(n = 0; n<4;n++)
                        if(carta_existe(simulacao->mao[jog],n,v)){
                            jogadas[count] = add_carta(jogadas[count],n,v);
                            n = 4;
                        }
                }
                else{
                    if(sametype != 0){
                        jogadas[count] = add_carta(jogadas[count], tempN[count2],v);
                        count++;
                    }
                    else{
                        for(n = 0; n<4;n++)
                            if(carta_existe(simulacao->mao[jog],n,v)){
                                jogadas[count] = add_carta(jogadas[count],n,v);
                                n = 4;
                                count++;
                            }
                    }
                }
            }
        }
    return count;
}



long long int flushpos (DATABASE * simulacao, int jog, int sametype, MAO jogadas[40], int count) {
    int i = 0;
    int naipe[4] = {0};
    int max = 0;
    int c = 0;
    int v;
    MAO temp = simulacao->mao[jog];
    separa_nap(temp, naipe);
    if(sametype != 0){
        max= maior_carta_mao(simulacao->jogada);
        i = max / 13;
        if(naipe[i]>4){
            for(v = 12; v >= 0; v--)
                if (carta_existe(temp,i,v)){
                    if(v > max % 13){
                        jogadas[count] = add_carta(0,i,v);
                        c = 1;
                        for(v--; v >= 0; v--)
                            if(carta_existe(temp,i,v)){
                                jogadas[count] = add_carta(jogadas[count],i,v);
                                c++;
                                if(c == 5){
                                    c = 0;
                                    i++;
                                    count++;
                                    v = -1;
                                    break;
                                }
                            }
                    }
                	else
                        i++;
                }
        }
        else{
            i++;
        }
        for(; i< 4; i++){
            if(naipe[i] > 4){
                jogadas[count] = 0;
                for(v = 0; v < 13; v++){
                    if (carta_existe(temp,i,v)){
                        jogadas[count] = add_carta(jogadas[count],i,v);
                        c++;
                        if(c==5){
                            count++;
                            c = 0;
                            break;
                        }
                    }
                }
            }
        }
    }
    else{
        i = 0;
        while(i < 4){
            if(naipe[i]>4){
                c = 0;
                jogadas[count] = 0;
                for(v = 0; v < 13 && c < 5; v++)
                    if (carta_existe(temp,i,v)){
                        jogadas[count] = add_carta(jogadas[count],i,v);
                        c++;
                        if(c == 5){
                            c = 0;
                            i++;
                            count++;
                            break;
                        }
                    }
            }
            else
                i++;
        }
    }
    return count;
}


int fullhousepos (DATABASE * simulacao, int jog, int sametype, MAO jogadas[40], int count) {
    int rank[13] = {0};
    int rank2[13] = {0};
    int i = 0, v, n;
    int c = 0;
    MAO mao = simulacao->mao[jog], temp = 0;
    separa_val(mao, rank);
    if(sametype != 0){
        separa_val(simulacao->jogada, rank2);
        for(i = 0; i < 13; i++)
            if(rank2[i] == 3){
                ++i;
                break;
            }
    }
    for(; i < 13; i++){
        temp = 0;
        if(rank[i] > 2){
            for(n = 0; n < 4; n++)
                if(carta_existe(mao, n, i)){
                    temp = add_carta(temp,n,i);
            		c++;
                    if(c == 3){
                        c = 0;
                        n = 4;
                    }
                }
            for(v = 0; v < 13; v++)
                if(rank[v] > 1 && v != i){
                    jogadas[count] = temp;
                    for(n = 0; n<4; n++)
                        if(carta_existe(mao,n,v)){
                            jogadas[count] = add_carta(jogadas[count],n,v);
                            c++;
                            if(c == 2){
                                c = 0;
                                count++;
                                n = 4;
                            }
                        }
                }
        }
    }
    return count;
}


int fourofakindpos (DATABASE * simulacao, int jog, int sametype,MAO jogadas[40], int count) { /*sametype: 1-> a última jogada foi um four of a kind 0: foi uma jogada de rank inferior */
    MAO temp = 0;
    int n,v, ind;
    int i = 0;
    int rank[13] = {0};
    separa_val(simulacao->mao[jog], rank);
    if(sametype != 0){
        i = (maior_carta_mao(simulacao->jogada) % 13) + 1;
    }
    for(; i < 13; i++)
        if(rank[i] == 4){
            jogadas[count] = 0;
            temp = add_carta(temp,0,i);
            temp = add_carta(temp,1,i);
            temp = add_carta(temp,2,i);
            temp = add_carta(temp,3,i);
            for(ind = 0; ind < 52;ind++){
                n = ind / 13;
                v = ind % 13;
                if(carta_existe(simulacao->mao[jog],n,v) && v != i){
                    jogadas[count] = add_carta(temp,n,v);
                    count++;
                }
            }
        }
    return count;
}


long long int straightflushpos (DATABASE * simulacao, int jog, int sametype,MAO jogadas[40], int count) {
    MAO temp = 0;
    int n=3;
    int c = 0;
    int naipe[4] = {0};
    separa_nap(simulacao->mao[jog], naipe);
    while(n>=0) {
        if(naipe[n]>4) {
            int v;
            for(v=12; v>=0; v--) {
                if(carta_existe(simulacao->mao[jog], n, v)) temp = add_carta(temp, n, v);
            }
             c = straightpos2(temp,simulacao,jog,sametype,jogadas,0);
            if(temp==0) n--;
            else {
                count += c;
                n = -1;
            }
        }
        else n--;
    }
    return count;
}

void simula_maos(DATABASE * data){
    int ind, jog;
    int i,n,v;
    int num[4] = {13,13,13,13};
    int flag = 0;
    MAO atribuir=0x000fffffffffffff;
    for(jog = 1; jog<4; jog++)
        data->mao[jog] = 0;
    
    for(ind = 0;ind < 52; ind ++){
        n = ind / 13;
        v = ind % 13;
        for(i = 0; i < 3; i++){
            if(carta_existe(data->usadas[i+1],n,v)){
                atribuir = rem_carta(atribuir, n, v);
                num[i]--;
            }
            else if(carta_existe(data->mao[0],n,v) ||carta_existe(data->usadas[0],n,v) )
                atribuir = rem_carta(atribuir, n, v);
        }
    }
    for (ind = 0; ind < 52 ; ind ++){
        n = ind / 13;
        v = ind % 13;
        if(carta_existe(atribuir,n,v)){
            do{
                flag = 0;
            	jog = rand()%3;
                if(num[jog] > 0){
                    data->mao[jog+1] = add_carta(data->mao[jog+1],n,v);
                    num[jog]--;
                    flag = 1;
                }
            }while(flag == 0);
        }
    }
}


int jogadas5(DATABASE * simulacao, int jog, MAO jogadas[40]){
    int r = 0;
    int count = 0;
    if(simulacao->jogada != 0)
        r = tipo_comb_five(simulacao->jogada);
    if(r <= 1){
        if(r==1)
    		count = straightpos(simulacao,jog,1, jogadas, count);
    	else
            count = straightpos(simulacao,jog,0, jogadas, count);
    }
    
    if(r<=2){
        if(r==2)
            count = flushpos(simulacao,jog,1, jogadas, count);
        else
            count = flushpos(simulacao,jog,0, jogadas, count);
    }
    if(r<=3){
        if(r==3)
            count = fullhousepos(simulacao,jog,1, jogadas, count);
        else
            count = fullhousepos(simulacao,jog,0, jogadas, count);
    }
    if(r<=4){
        if(r==4)
            count = fourofakindpos(simulacao,jog,1, jogadas, count);
        else
            count = fourofakindpos(simulacao,jog,0, jogadas, count);
    }
    if(r<=5){
        if(r==5)
            count = straightflushpos(simulacao,jog,1, jogadas, count);
        else
            count = straightflushpos(simulacao,jog,0, jogadas, count);
    }

    count++; /*adiciona o passar, já está a zero*/
    return count;
}

int jogadasN(DATABASE * simulacao, int jog, MAO jogadas [4][40], int nc){    int n, v = 0, i, temp_naipe[4]={0}, count = 0, a;
    MAO estado = simulacao->mao[jog];
    int max = maior_carta_mao(simulacao->jogada);
    if (max==-1) v=0;
    else v=max%13;
    for(; v < 13; v++){
        i = 0;
        for(n = 0; n < 4; n++)
            if(carta_existe(estado,n,v)){
                temp_naipe[i] =n;
                i++;
            }
        if(i >= (nc+1) && (v > max % 13 || temp_naipe[i - 1] > max / 13)){
            jogadas[nc][count] = 0;
            for(a = 0, i = i -1; a <= nc; i--, a++)
                jogadas[nc][count] = add_carta(jogadas[nc][count], temp_naipe[i], v);
            count++;
        }
    }
    jogadas[nc][count] = 0;
    count++;
    return count;
}

void jogadas_possiveis(DATABASE * simulacao, int counter[4], int jog, MAO jogadas[4][40]){
    int nc;
    if(simulacao->nc == 0){
   	 	counter[3] = jogadas5(simulacao, jog, jogadas[3]);
        for(nc = 0; nc < 3; nc++)
    		counter[nc] = jogadasN(simulacao, jog, jogadas, nc);
    }
    else{
    	if(simulacao->nc == 4)
        	counter[3] = jogadas5(simulacao, jog, jogadas[3]);
    	else
    		counter[simulacao->nc - 1] = jogadasN(simulacao, jog, jogadas, (simulacao -> nc)-1);
    }
}

/*

#####################################################################################################################################

 */


MCtree createTree(MAO mao){
    int i, nc;
    MCtree tree = malloc(sizeof(searchTree));
    tree -> t = (long)0;
    tree -> r = (float)0;
    tree -> estado = mao;
    tree -> play = 0;
    tree->selected = 0;
    for(i = 0; i < 40; i++)
        for(nc = 0; nc < 4; nc++)
        	tree -> nextN[nc][i] = NULL;
    tree->prev = NULL;
    return tree;
}

MCtree addNodo(MAO mao, MCtree previous){
    int i,nc;
    MCtree tree;
	tree = malloc(sizeof(searchTree));
    tree -> t = 0;
    tree -> r = 0;
    tree -> play = 0;
    tree -> estado = mao;
    tree -> selected = 0;
    for(i = 0; i < 40; i++)
        for(nc = 0; nc < 4; nc++)
            tree -> nextN[nc][i] = NULL;
    tree->prev = previous;
    return tree;
}

void rewardF(MCtree temp, DATABASE * simulacao){
    int i, n, v;
    float count = 0.0;
    for(i = 0; i < 52; i++){
        n = i / 13;
        v = i % 13;
        if(carta_existe(simulacao -> mao[0],n,v))
            count++;
    }
    if(count <= 9)
        count = 1.0 - (count * 0.03);
    else if(count > 9 && count < 13)
        count = 1.0 - (count * 0.06);
    else
        count = 0.0;
    while(temp != NULL){
        temp->r += count;
        temp->t++;
        temp = temp->prev;
    }
}



MCtree treePolicy(MCtree tree, DATABASE * simulacao, int * flag2){
    int counter[4] = {0};
    float UCT_value = 0.0;
    float max = -1.0;
    int index = 0;
    int count1 = 0, count2 = 0;
    int nc = (simulacao ->nc) - 1;
    int wi = 0, ni = 0, t = 0;
    MAO jogadas[4][40];
    MAO jogada_max = 0;
    memset(jogadas,0,8 * 4 * 40);
	jogadas_possiveis(simulacao,counter,0,jogadas);
    if(nc >= 0){
        count1 = 0;
        while(count1 < counter[nc]){
            for(count2 = 0; count2 < 40; count2++){
                if(tree->nextN[nc][count2] == NULL){
                    tree->nextN[nc][count2] = addNodo(jogadas[nc][count1], tree);
                    simulacao->mao[0] = (simulacao->mao[0]) ^ (jogadas[nc][count1]);
                    if(jogadas[nc][count1] == 0)
                        simulacao->passar++;
                    else{
                        simulacao->passar = 0;
                        simulacao->jogada = jogadas[nc][count1];
                    }
                    if(simulacao->passar == 3){
                        simulacao->nc = 0;
                        simulacao->jogada = 0;
                    }
                    simulacao->usadas[0] = simulacao->usadas[0] | (jogadas[nc][count1]);
                    flag2[0] = 1;
                    return tree->nextN[nc][count2];
                }
                else if((jogadas[nc][count1]) == ((tree->nextN[nc][count2])->estado)){
                    wi = (tree->nextN[nc][count2])->r;
                    ni = (tree->nextN[nc][count2])->t;
                    t = tree->t;
                	UCT_value = (float)(((wi) / (float)(ni)) + (1.4142136 * (sqrt(log(t) / (ni)))));
                    count1++;
                    if(UCT_value > max){
                        max = UCT_value;
                        index = (nc * 40) + count2;
                        jogada_max = jogadas[nc][count1];
                    }
                    count2 = 40;
                }
            }
    	}
    }
    else{
        for(nc = 0; nc < 4; nc++){
            count1 = 0;
            while(count1 < counter[nc]){
                for(count2 = 0; count2 < 40; count2++){
                    if(jogadas[nc][count1] != 0){
                        if(tree->nextN[nc][count2] == NULL){
                            simulacao->nc = nc + 1;
                            simulacao->passar = 0;
                            simulacao->jogada = jogadas[nc][count1];
                            simulacao->mao[0] = (simulacao->mao[0]) ^ (jogadas[nc][count1]);
                            tree->nextN[nc][count2] = addNodo(jogadas[nc][count1], tree);
                            simulacao->usadas[0] = simulacao->usadas[0] | jogadas[nc][count1];
                            flag2[0] = 1;
                            return tree->nextN[nc][count2];
                        }
                        else if((jogadas[nc][count1])== ((tree->nextN[nc][count2])->estado)){
                            wi = (tree->nextN[nc][count2])->r;
                            ni = (tree->nextN[nc][count2])->t;
                            t = tree->t;
                            UCT_value = (float)(((wi) / (float)(ni)) + (1.4142136 * (sqrt(log(t) / (ni)))));
                            count1++;
                            if(UCT_value > max){
                                max = UCT_value;
                                index = (nc * 40) + count2;
                                jogada_max = jogadas[nc][count1];
                                simulacao->nc = nc + 1;
                            }
                            count2 = 40;
                        }
                    }
                    else
                        count1++;
                }
            }
        }
    }
    if(jogada_max == 0)
        simulacao->passar++;
    else{
        simulacao->jogada = jogada_max;
        simulacao->passar = 0;
        simulacao->usadas[0] = simulacao->usadas[0] | jogada_max;
    }
    if(simulacao->passar == 3){
        simulacao->nc = 0;
        simulacao->jogada = 0;
    }
    tree = tree->nextN[index/40][index%40];
    simulacao->mao[0] = jogada_max ^ (simulacao->mao[0]);
    return tree;
}



int defaultPolicy(MCtree node, DATABASE * simulacao, int * flag2){
    int jogo = 0, counter[4] = {0}, flag = 0, j = 0;
    MAO jogadas[4][40];
    int i, nc, round = 0;
    memset(jogadas,0,sizeof (jogadas[0][0]) * 4 * 40);
    for(;j<4; j++)
        if(simulacao->mao[j] == 0){
            rewardF(node,simulacao);
            return 1;
        }
    if((node->prev)->prev == NULL)
        simula_maos(simulacao);
    j = 1;
    if(flag2[0] == 0)
        jogo = 1;
    do{
        if(j == 3)
            round = 1;
        jogadas_possiveis(simulacao, counter, j, jogadas);
        if(simulacao->nc == 0){
            do{
                nc = (rand() % 4);
                i = rand() % counter[nc];
            }while(jogadas[nc][i] <= 0);
            
            simulacao->mao[j] = simulacao->mao[j]  ^ jogadas[nc][i];
            if(simulacao->mao[j] == 0){
                jogo = 1;
                flag = 1;
            }
            else{
                j = (j + 1) % 4;
                simulacao->jogada = jogadas[nc][i];
                simulacao->nc = nc + 1;
                simulacao->usadas[j] = simulacao->usadas[j] | simulacao->jogada;
            }
            simulacao -> passar = 0;
        }
        else{
            nc = simulacao->nc - 1;
            i = rand() % counter[nc];
            if(jogadas[nc][i] == 0){
                simulacao->passar++;
                if(simulacao->passar == 3){
                    simulacao -> nc = 0;
                    simulacao -> jogada = 0;
                }
            }
            else{
                simulacao->mao[j] = simulacao->mao[j] ^ jogadas[nc][i];
                if(simulacao->mao[j] == 0){
                    jogo = 1;
                    flag = 1;
                }
                else{
                    j = (j + 1) % 4;
                    simulacao->jogada = jogadas[nc][i];
                    simulacao->usadas[j] = simulacao->usadas[j] | simulacao->jogada;
                }
                simulacao->passar = 0;
            }
        }
    }while(jogo == 0 || round == 0);
    if (flag != 0){
        rewardF(node, simulacao);
    }
    return flag;
}

void free_nodes_aux(MCtree tree){
    int nc,i;
    for(nc = 0; nc < 4; nc++)
        for(i = 0; ((tree->nextN[nc][i]) != NULL) &&  (i < 40);i++){
            free_nodes_aux(tree->nextN[nc][i]);
            free(tree->nextN[nc][i]);
        }
}
void free_nodes(MCtree tree, MCtree new_tree){
    int nc,i;
    for(nc = 0; nc < 4; nc++)
        for(i = 0; ((tree->nextN[nc][i]) != NULL) &&  (i < 40);i++)
            if((tree->nextN[nc][i]) != new_tree){
                free_nodes_aux(tree->nextN[nc][i]);
                free(tree->nextN[nc][i]);
            }
    free(tree);
}


char naiperev (int c) {
    char r;
    switch (c) {
        case 0:
            r = 'D';
            break;
        case 1:
            r = 'C';
            break;
        case 2:
            r = 'H';
            break;
        case 3:
            r = 'S';
            break;
    }
    return r;
}

char valorrev (int c) {
    char r;
    if(c<=6) r = c+'3';
    else
        switch (c) {
            case 7:
                r = 'T';
                break;
            case 8:
                r = 'J';
                break;
            case 9:
                r = 'Q';
                break;
            case 10:
                r = 'K';
                break;
            case 11:
                r = 'A';
                break;
            case 12:
                r = '2';
                break;
        }
    return r;
}

void convertejogstr(MAO mao, char * output){
    int ind, v ,n,i=0,flag = 0;
    for (ind=0; ind<52; ind++){
        v= ind %13;
        n= ind /13;
        if (carta_existe (mao,n,v)){
            flag = 1;
            output[i] = valorrev(v);
            output[i+1]= naiperev(n);
            output[i+2]= ' ';
            i +=3;
        }
    }
    output[i-1] = '\0';
    if(flag == 0)
        strcpy(output,"PASSO");
    
}

MCtree choosePlay(MCtree tree, DATABASE * data, int nc_counter[4]){
    int nc;
    int max_t = 0;
    float max_r = 0.0;
    int counter_tree = 0, counter_jogadas = 0;
    MAO jogadas[4][40];
    memset(jogadas,0,8*4*40);
    MCtree play_t;
    MCtree play_r;
    MCtree play;
    int nc_t = -1, nc_r = -1;
    int counter[4] = {0};
    int i = 0;
    
    jogadas_possiveis(data,counter,0,jogadas);
    play_t = NULL;
    play_r = NULL;
    play = NULL;
    for(nc = 0; nc < 4; nc+= 1){
        counter_jogadas = 0;
        counter_tree = 0;
    	while((counter_jogadas < counter[nc]) && (counter_tree < 40) && ((tree->nextN[nc][counter_tree]) != NULL))
            for(counter_tree = 0; ((tree->nextN[nc][counter_tree]) != NULL) && (counter_tree < 40); counter_tree += 1)
                if(jogadas[nc][counter_jogadas] == (tree->nextN[nc][counter_tree]-> estado)){
                    if ((tree->nextN[nc][counter_tree]->t) > max_t){
                        max_t =tree->nextN[nc][counter_tree]->t;
                        play_t = tree->nextN[nc][counter_tree];
                        nc_t = nc;
                    }
                    if((tree->nextN[nc][counter_tree]->r) > max_r){
                        max_r = tree->nextN[nc][counter_tree]->r;
                        play_r = tree->nextN[nc][counter_tree];
                        nc_r = nc;
                    }
                    counter_jogadas++;
                    if(counter_jogadas == counter[nc])
                        nc++;
                }
    }
    if(play_r == play_t){
        play = play_r;
        nc_counter[nc_t]++;
        for(i = 0; i< 4; i++)
            if(i!=nc_r)
                nc_counter[i] = 0;
    }
    else if((play_r != NULL) && (play_r->t) > 1000){
        play = play_r;
        nc_counter[nc_r]++;
        for(i = 0; i< 4; i++)
            if(i!=nc_r)
                nc_counter[i] = 0;
    }
    else{
        play = play_t;
        if(play != NULL){
            nc_counter[nc_t]++;
            for(i = 0; i< 4; i++)
                if(i!=nc_t)
                    nc_counter[i] = 0;
        }
    }
    return play;

}

void clean_plays(MCtree tree, int pass_counter,int nc_counter[4]){
    int counter_tree, nc;
    if(pass_counter > 0)
        for(nc = 0; nc < 4; nc+= 1){
            for(counter_tree = 0; (tree->nextN[nc][counter_tree] != NULL) && (counter_tree < 40); counter_tree++)
                if(tree->nextN[nc][counter_tree]->estado == 0){
                    tree->nextN[nc][counter_tree] -> t = 0;
                    tree->nextN[nc][counter_tree] -> r = 0;
                }
        }
    for(nc = 0; nc < 4; nc+= 1){
        if(nc_counter[nc]>1)
            for(counter_tree = 0; (tree->nextN[nc][counter_tree] != NULL) && (counter_tree < 40); counter_tree++){
                tree->nextN[nc][counter_tree] -> t = 0;
                tree->nextN[nc][counter_tree] -> r = 0;
            }
    }
}

void checkFirst(MCtree tree, DATABASE * data){
    int counter_tree, nc;
    for(nc = 0; nc < 4; nc+= 1){
        for(counter_tree = 0; (tree->nextN[nc][counter_tree] != NULL) && (counter_tree < 40); counter_tree++)
            if((tree->nextN[nc][counter_tree]->estado == 0) || (carta_existe(tree->nextN[nc][counter_tree]->estado,0,0) == 0)){
                    tree->nextN[nc][counter_tree]->t = 0;
                    tree->nextN[nc][counter_tree]->r = 0;
            }
	}
    data->firstplay = 2;
}


int main(){
    char input[100];
    int flag = 0, i;
    int flag2[1] = {0};
    char output[100];
    int n, v;
    int io_count = 0;
    int counter = 0;
    int jog = 3;
    int jog_temp = 0;
    int a = 0;
    int pass_counter = 0;
    int nc_counter[4] = {0};
    int flag3 = 0;
    MAO mao_temp = 0;
    MAO usadas_temp[3] = {0};
    MCtree temp;
    char mao[13][2];
    DATABASE data = {{0},0,0,0,{0},0};
    DATABASE simulacao;
    MCtree tree = NULL;
    clock_t start;
    
    
    memset(mao,0,26);
    srand(time(0));
    fgets(input, 100, stdin);
    while(input[0] != 'A'){
        switch(input[0]){
            case 'J':
                if(input[3] == 'A'){
                    start = clock();
                    if(data.firstplay == 0){
                        data.firstplay = 1;
                        for(jog_temp = jog_temp -1, counter = 0; counter <= jog_temp; counter++)
                            data.usadas[counter +(3-jog_temp)] = usadas_temp[counter];
                        if(usadas_temp[0] != 0)
                            data.firstplay = 2;
                    }
                    clean_plays(tree, pass_counter, nc_counter);
                    while(((clock()-start)/CLOCKS_PER_SEC) < 1.7){
                        simulacao = data;
                        while(flag == 0){
                            temp = treePolicy(tree, &simulacao, flag2);
                            flag = defaultPolicy(temp, &simulacao, flag2);
                            flag2[0] = 0;
                            a++;
                        }
                        flag = 0;
                    }
                    jog = 1;
                    if(data.firstplay != 2)
                        checkFirst(tree, &data);
                    temp = choosePlay(tree,&data, nc_counter);
                    if(temp != NULL){
                    	convertejogstr(temp->estado,output);
                        flag3 = 1;
                        printf("%s\n", output);
                    }
                    else{
                        strcpy(output, "PASSO");
                        data.firstplay = 0;
                        jog_temp = 0;
                        data.passar = 0;
                    }
                    if(flag3 != 0){
                        if(output[0] != 'P')
                            fgets(input, 100, stdin);
                        else
                            input[0] = 'O';
                        if(input[0] == 'O'){
                            if(temp->estado == 0){
                                data.passar++;
                                pass_counter++;
                            }
                            else{
                                data.passar = 0;
                                pass_counter = 0;
                                data.jogada = temp->estado;
                                data.nc = 0;
                                for(i = 0; i < 52; i++){
                                    n = i / 13;
                                    v = i % 13;
                                    if(carta_existe(temp->estado,n,v))
                                        data.nc++;
                                }
                            }
                            if(data.passar == 3){
                                data.nc = 0;
                                data.jogada = 0;
                            }
                            free_nodes(tree,temp);
                            tree = temp;
                            tree->prev = NULL;
                            data.usadas[0] = data.usadas[0] | tree-> estado;
                            data.mao[0] = (data.mao[0]) ^ (tree->estado);
                        }
                        else if(input[0] == 'N'){
                            data.passar++;
                            pass_counter++;
                            if(data.passar == 3){
                                data.nc = 0;
                                data.jogada = 0;
                            }
                        }
                    }
                }
                else{
                    mao_temp = 0;
                    io_count = sscanf(input, JOGADA, &mao[0][0],&mao[0][1],&mao[1][0],&mao[1][1],&mao[2][0],&mao[2][1],&mao[3][0],&mao[3][1],&mao[4][0],&mao[4][1],&mao[5][0],&mao[5][1],&mao[6][0],&mao[6][1],&mao[7][0],&mao[7][1],&mao[8][0],&mao[8][1],&mao[9][0],&mao[9][1],&mao[10][0],&mao[10][1],&mao[11][0],&mao[11][1],&mao[12][0],&mao[12][1]);
                    for(i = 0; i < io_count/2; i++){
                        if(mao[i][0] >= '3' && mao[i][0] <='9')
                            v = mao[i][0] - '3';
                        else{
                            switch(mao[i][0]){
                                case '2':
                                    v = 12;
                                    break;
                                case 'A':
                                    v = 11;
                                    break;
                                case 'K':
                                    v = 10;
                                    break;
                                case 'Q':
                                    v = 9;
                                    break;
                                case 'J':
                                    v = 8;
                                    break;
                                case 'T':
                                    v = 7;
                                    break;
                            }
                        }
                        switch (mao[i][1]){
                            case 'D':
                                n = 0;
                                break;
                            case 'C':
                                n = 1;
                                break;
                            case 'H':
                                n = 2;
                                break;
                            case 'S':
                                n = 3;
                                break;
                        }
                        mao_temp = add_carta(mao_temp,n,v);
                    }
                    if(data.firstplay != 0){
                        jog = (jog + 1) % 4;
                        data.usadas[jog] = data.usadas[jog] | mao_temp;
                    }
                    else{
                        usadas_temp[jog_temp] = mao_temp;
                        jog_temp++;
                    }
                    data.jogada = mao_temp;
                    data.nc = i;
                    if(data.nc == 5)
                        data.nc = 4;
                    data.passar = 0;
                    flag3 = 1;
                }
                break;
            case 'P':
                data.passar++;
                if(data.firstplay == 0){
                    if(jog_temp != 0)
                    	jog_temp++;
                }
                else{
                    jog = (jog + 1) % 4;
                }
                if(data.passar == 3){
                    data.nc = 0;
                    data.jogada = 0;
                }
                break;
            case 'M':
                sscanf(input, MAO_INICIAL, &mao[0][0],&mao[0][1],&mao[1][0],&mao[1][1],&mao[2][0],&mao[2][1],&mao[3][0],&mao[3][1],&mao[4][0],&mao[4][1],&mao[5][0],&mao[5][1],&mao[6][0],&mao[6][1],&mao[7][0],&mao[7][1],&mao[8][0],&mao[8][1],&mao[9][0],&mao[9][1],&mao[10][0],&mao[10][1],&mao[11][0],&mao[11][1],&mao[12][0],&mao[12][1]);
                for(i = 0; i < 13; i++){
                    if(mao[i][0] >= '3' && mao[i][0] <='9')
                        v = mao[i][0] - '3';
                    else{
                        switch(mao[i][0]){
                        	case '2':
                                v = 12;
                                break;
                            case 'A':
                                v = 11;
                                break;
                            case 'K':
                                v = 10;
                                break;
                            case 'Q':
                                v = 9;
                                break;
                            case 'J':
                                v = 8;
                                break;
                            case 'T':
                                v = 7;
                                break;
                        }
                    }
                    switch (mao[i][1]){
                        case 'D':
                            n = 0;
                            break;
                        case 'C':
                            n = 1;
                            break;
                        case 'H':
                            n = 2;
                            break;
                        case 'S':
                            n = 3;
                            break;
                    }
                    data.mao[0] = add_carta(data.mao[0],n,v);
                }
                tree = createTree(data.mao[0]);
                temp = tree;
                break;
            
        }
    	fgets(input, 100, stdin);
    }
    return 0;
}
