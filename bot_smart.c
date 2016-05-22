


typedef long long int MAO;
struct database{
    MAO mao[2];
    MAO last_play;
    int nc; //número de cartas da ronda
    int passar;
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

int naipe (char c) {
	int r;
	switch c {
		case 68: r = 0;
		case 67: r = 1;
		case 72: r = 2;
		case 83: r = 3;
	}
}

int valor (char c) {
	int r;
	if(c>50 && c<58) r = c-51;
	else
		switch c {
			case 84: r = 7;
			case 74: r = 8;
			case 81: r = 9;
			case 75: r = 10;
			case 50: r = 11;
			case 65: r = 12;
	}
	return r;
}

MAO converte_s_m (char * s) {
	MAO r = 0;
	int i,n,v;
	if (s[0]=='M') {
        data.mao[0]=0;add_carta
		for(i=4; i<41; i+=3){
            v= valor(s[i]);
            n= naipe(s[i+1]);
            add_carta(r,n,v);
        }
	}
    for (i=0; i<52; i++){}
        v= i %13;
        n= i /13;
        if carta_existe(r,n,v)
            rem_carta (data->mao[1],n,v);
    }
    return r;
}

int main() {

}