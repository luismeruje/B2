#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long long int MAO;
struct database{
    MAO mao[2];
    MAO last_play;
    int nc; //número de cartas da ronda
    int passar;
    int nm[4];
    int combination[3];
};

typedef struct database DATABASE;

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

//r: 1 => straight; 2 => flush; 3 => fullhouse; 4 => fourofakind; 5 => straightflush
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

void atualizastraight(MAO mao, int y[3]) { //modifica talvez o data.last_play... pode influenciar
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

void preenchejogada (MAO mao, int y[3], int nc){
    int m, i, ind;
    if (nc == 5) {
        int rank[13] = {0};
        m = tipo_comb_five(mao);
        y[0] = m;
        if (m == 1 || m == 5) atualizastraight(mao, y);
        else  if (m == 2) {
                ind = maior_carta_mao(mao);
                y[1] = ind % 13;
                y[2] = ind / 13;
              }
              else {
                separa_val(mao, rank);
                for(i=0; i<13; i++) if (rank[i] >= 3) break;
                y[1] = i;
                y[2] = 0;
              }       
    }
    else {
        y[0] = 0;
        ind = maior_carta_mao(mao);
        y[1] = ind % 13;
        y[2] = ind / 13;
    }
}

int naipe (char c) {
    int r;
    switch (c) {
        case 68: r = 0; break;
        case 67: r = 1; break;
        case 72: r = 2; break;
        case 83: r = 3;
    }
    return r;
}

int valor (char c) {
    int r;
    if(c>50 && c<58) r = c-51;
    else
        switch (c) {
            case 84: r = 7; break;
            case 74: r = 8; break;
            case 81: r = 9; break;
            case 75: r = 10; break;
            case 65: r = 11; break;
            case 50: r = 12;
    }
    return r;
}

int calculalixosdt (MAO mao){
    int i,n,v, z,r=0;
    int rank[13]={0};
    separa_val(mao, rank);
    for (i=0; i<5 ; i++)
        if (rank[i] == 1) r += 1;
        if (rank[i] == 2) {
            for (z = 0; z<13; z++)
                if (rank[z] >= 3) break;
            if (z == 13) r += 2;
        }
        if (rank[i] == 3)
        {
            for (z = 0; z<13; z++)
                if ((rank [z] >= 2) && (z !=i)) break;
            if (z == 13) r += 3;
        }
        if (rank[i] == 4) {
            for (z = 0; z<13;z++)
                if ((rank [z] != 0) && (z !=i)) break;
            if (z == 13) r += 4;
        }
    return r;
}

int calcontrolt (MAO mao, MAO restantes, MAO last_play){
    MAO outras = restantes + last_play;
    int rank1[13] = {0};
    int rank2[13] = {0};
    int r = 0, i, z;
    separa_val(mao, rank1);
    for (i=12; i>=0; i--) if (rank1[i] >= 3) break;
    if (i != -1) {
        separa_val(outras, rank2);
        for (z=12; z>0; z--) if (rank2[z] >= 3) break;
    }
    if (i > z) r++;
    while (i > z) {
        i--;
        if (rank1[i] >= 3) r++;
    }
    return r;
}

int calcontrold (MAO mao, MAO restantes, MAO last_play) {
    MAO outras = restantes + last_play;
    int i = 12, p = 12, r = 0;
    int rank1[13] = {0};
    int rank2[13] = {0};
    separa_val(mao, rank1);
    separa_val(outras, rank2);
    while (rank2[p]<2 && i>-1) p--;
    while (rank1[i]<2 && i>-1) i--;
    while (i >= p) {
        while (rank1[i]<2) i--;
        r += (i > p || (i == p && carta_existe(mao, 3, i))) ? (1) : (0);
        i--;
    }
    return r;
}

int calcontrols (MAO mao, MAO restantes, MAO last_play){
    MAO outras = restantes + last_play;
    int rank[13] = {0};
    int i, m, mn, mv, n, v;
    int r = 0;
    m = maior_carta_mao(outras);
    mn = m / 13;
    mv = m % 13;
    for(i=12, n=3; (i>mv || (i==mv && n>mn));) {
        n = (n < 0)? (3) : (n);
        i -= (n < 0)? (1) : (0);
        if (carta_existe(mao,n,i)) r += 1;
        n--;
    }
    n = (n < 0)? (3) : (n-1);
    i -= (n < 0)? (1) : (0);
    while (carta_existe(mao,n,i)) { 
        n = (n < 0)? (3) : (n-1);
        i -= (n < 0)? (1) : (0);
        if (carta_existe(mao,n,i)) r += 1;
    }
    return r;
}

char naiperev (int c) {
    char r;
    switch (c) {
        case 0: r = 68;
        case 1: r = 67;
        case 2: r = 72;
        case 3: r = 83;
    }
    return r;
}

char valorrev (int c) {
    char r;
    if(c<6) r = c+51;
    else
        switch (c) {
            case 7: r = 84;
            case 8: r = 74;
            case 9: r = 81;
            case 10: r = 75;
            case 11: r = 50;
            case 12: r = 65;
    }
    return r;
} 

void convertejogstr(MAO mao, char * output){
    int ind, v ,n,i=0;
    for (i=0; ind<52; ind++){
        v= ind %13;
        n= ind /13;
        if (carta_existe (mao,n,v)){
            output[i] = valorrev(v);
            output[i+1]= naiperev(n);
            output[i+2]= ' ';
            i +=3;
        }
    }
    output[i-1] = '\0';
}

void conv_jog (char * s, DATABASE * data, int c) {
    int i, n, v;
    data->last_play = 0;
    data->nc = 0;
    for(i=6; s[i-1] != '\0'; i += 3) {
        v = valor(s[i]);
        n = naipe(s[i+1]);
        data->last_play = add_carta(data->last_play, n, v);
        data->mao[1] = rem_carta(data->mao[1], n, v);
        data->nm[c]--;
        data->nc++;
    }
    preenchejogada(data->last_play, data->combination, data->nc);
}

void converte_s_m (char * s, DATABASE * data) {
	int i, n, v;
	for(i=4; i<41; i+=3){
        v = valor(s[i]);
        n = naipe(s[i+1]);
        data->mao[0] = add_carta(data->mao[0], n, v);
        data->mao[1] = rem_carta(data->mao[1], n, v);
    }
}

MAO jogsinglecard (DATABASE * data, int control, int c) {
    int rank[13] = {0};
    int i, a, p, v, n;
    separa_val(data->mao[0], rank);
    int ncp[4];
    for (i=0; i<4; i++) {
        ncp[i] = data->nm[c];
        c = ((c+1)%4);
    }
    MAO temp = 0;
    if(control > 0) { // falta considerar quando algum jogador estiver a acabar
        p = calculalixosdt(data->mao[0]);
        if (p > 2) { // talvez ter em conta os pares ou trios
            a = maior_carta_mao(data->mao[0]);
            temp = add_carta(temp, a/13, a%13);
        }
        else {
            i = data->combination[1];
            for (v=i; v<13; v++) {
                if (rank[v] > 0) {
                    if (v > i) {
                        for(n=0; n<4; n++) 
                            if(carta_existe(data->mao[0], n, v)) {
                                temp = add_carta(temp, n, v);
                                break;
                            }
                    }
                    else {
                        for(n=data->combination[2]; n<4; n++)
                            if(carta_existe(data->mao[0], n, v)) {
                                temp = add_carta(temp, n, v);
                                break;
                            }
                    }
                }
            }
        } 
    }
    else {
        i = data->combination[1];
        for (int p = 1; p < 4; p++) if(ncp[p] < 3) break;
        if (p < 3) a = maior_carta_mao(data->mao[0]);
        if (p == 3) 
            for (v=i; v<13; v++) {
                if (rank[v] > 0) {
                    if (v > i) {
                        for(n=0; n<4; n++) 
                            if(carta_existe(data->mao[0], n, v)) {
                                temp = add_carta(temp, n, v);
                                break;
                            }
                    }
                    else {
                        for(n=data->combination[2]; n<4; n++)
                            if(carta_existe(data->mao[0], n, v)) {
                                temp = add_carta(temp, n, v);
                                break;
                            }
                    }
                }
            }
    }
    return temp;
}

MAO jogdoublecard (DATABASE * data, int control, int c) {
    MAO temp = 0;
    int p, v, a, i, count;
    p = calculalixosdt(data->mao[0]);
    int rank1[13] = {0};
    separa_val(data->mao[0], rank1);
    if(control > 0 && p > 2) {
        for(p=13; p>=0; p--)
            if(rank1[p]>1) {
                for(v=0, count=0; (v<4 && count<2); v++)
                    if(carta_existe(data->mao[0], v, p)) {
                        temp = add_carta(temp, v, p);
                        count++;
                    }
            }
    }
    else {
        int rank2[13] = {0};
        separa_val(data->mao[1], rank2);
        i = data->combination[1];
        int ncp[4];
        for (i=0; i<4; i++) {
            ncp[i] = data->nm[c];
            c = ((c+1)%4);
        }
        for (p = 1; p < 4; p++) if(ncp[p] < 4) break;
        p = (p!=4) ? (13) : (12);
        a = (data->combination[2] == 3) ? (i+1) : (i); // caso a carta de espadas esteja na last_play não vale a pena testar se temos duplo nesse rank
        for(; a<p; a++) {
            if(rank1[a] > 1) {
                for(v=0, count=0; (v<4 && count<2); v++) 
                    if(carta_existe(data->mao[0], v, a)) {
                        temp = add_carta(temp, v, a);
                        count++;
                    }
            }
        }
    }
    return temp;
}

MAO jogtriplecard (DATABASE * data, int control, int c) {
    MAO temp = 0;
    int p, v, a, i, count;
    int rank1[13] = {0};
    int rank2[13] = {0};
    separa_val(data->mao[0], rank1);
    separa_val(data->mao[1], rank2);
    if(control > 0 && p > 2) {
        for(p=13; p>=0; p--)
            if(rank1[p]>2)
                for(v=0, count=0; (v<4 && count<3); v++)
                    if(carta_existe(data->mao[0], v, p)) {
                        temp = add_carta(temp, v, p);
                        count++;
                    }
    }
    else {
        i = data->combination[1];
        int ncp[4];
        for (i=0; i<4; i++) {
            ncp[i] = data->nm[c];
            c = ((c+1)%4);
        }
        for (p = 1; p < 4; p++) if(ncp[p] < 5) break;
        p = (p!=4) ? (13) : (12);
        for(a=i+1; a<p; a++) {
            if(rank1[a] > 2) {
                for(v=0, count=0; (v<4 && count<3); v++) 
                    if(carta_existe(data->mao[0], v, a)) {
                        temp = add_carta(temp, v, a);
                        count++;
                    }
            }
        }
    }
    return temp;
}

// MAO jogfivecard (DATABASE * data, int c) {

// }

MAO analisa_jog (DATABASE * data, int c) {
    MAO jogada = 0;
    int control, nc;
    if (data->passar == 3) {

    }
    else {
        nc = data->nc;
        switch(nc) {
            case 1: {
                control = calcontrols(data->mao[0], data->mao[1], data->last_play);
                jogada = jogsinglecard(data, control, c);
            }
            case 2: {
                control = calcontrold(data->mao[0], data->mao[1], data->last_play);
                jogada = jogdoublecard(data, control, c);
            }
            case 3: {
                control = calcontrolt(data->mao[0], data->mao[1], data->last_play);
                jogada = jogtriplecard(data, control, c);
            }
        }
    }
    return jogada;
}

int main() {
    DATABASE data = {{0,4503599627370495},0,0,0,{13,13,13,13},{0,0,0}};
    MAO jogada = 0;
    int c = 0, i = 0;
    char input[100];
    char output[100];
    fgets(input, 100, stdin);
    converte_s_m(input, &data);
    while(input[0] != 'A') {
        fgets(input, 100, stdin);
        if (input[0] == 'P') {
            data.passar++;
            c = (c+1)%4;
        }
        if (input[4] == 'U') {
            data.passar = 0;
            c = (c+1)%4;
            conv_jog(input, &data, c);
            printf("%lld\n", data.last_play);
        }
        if (input[3] == 'A') {
            jogada = analisa_jog(&data, c);
            if (jogada == 0) {
                data.passar++;
                printf("PASSO\n"); //retirar a nm[c] quantidade de cartas jogadas
            }
            else {
                convertejogstr(jogada, output);
                printf("%s", output);
                data.passar = 0;
            }
            c = (c+1)%4;
        }
        if (input[0] == 'O') {
            data.mao[0] -= jogada;
        }
    }
    return 0;
}