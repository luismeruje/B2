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

#define DATA "%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%lld_%d_%d_%d_%d"

typedef long long int MAO;
struct database{
    MAO mao[4];
    MAO selected;
    MAO jogadas[4];
    int play;
    int nc; //número de cartas a serem jogadas por cada jogador
    int passar;
    int inicio;
};

typedef struct database DATABASE;



//#################################UTILITÁRIOS#############################################

DATABASE STR2DATA(char * str){
    DATABASE data;
    sscanf(str, DATA,&(data.mao[0]),&(data.mao[1]),&(data.mao[2]),&(data.mao[3]),&(data.selected),&(data.jogadas[0]),&(data.jogadas[1]),&(data.jogadas[2]),&(data.jogadas[3]),&data.play,&data.nc,&data.passar,&data.inicio);
    return data;
}

void DATA2STR(char * str,DATABASE data){
    sprintf(str,DATA,data.mao[0],data.mao[1],data.mao[2],data.mao[3],data.selected,data.jogadas[0],data.jogadas[1],data.jogadas[2],data.jogadas[3],data.play,data.nc,data.passar,data.inicio);//TODO: a string com os lld's acho que dá para subsituir por uma palavra com um "define" no topo
}
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


int all_passed(DATABASE data){
    int result=0;
    if (data.passar>=3)
        result=1;
    return result;
}


void imprime_carta_imagem(char * path,int x, int y,int n,int v){
 	char *suit = NAIPES;
    char *rank = VALORES;
    printf("<image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" />\n", x, y, path, rank[v], suit[n]);
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

int quem_comeca(DATABASE data){
	int first,jog;
	for(jog=0;;jog++)
		if(carta_existe(data.mao[jog],0,0)){
			first = jog;
			break;
		}
	return first;
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
    sprintf(script,DATA,data.mao[0],data.mao[1],data.mao[2],data.mao[3],data.selected,data.jogadas[0],data.jogadas[1],data.jogadas[2],data.jogadas[3],data.play,data.nc,data.passar,data.inicio);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"350\" y = \"250\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_start.svg\" /></a>\n", script, path);
}

void imprime_passar (DATABASE data,char * path){
	char script[52000];
    data.selected = 0;
    data.jogadas[0]=0;
    data.play = 1;
    data.passar++;
    DATA2STR(script, data);
    printf("<a xlink:href = \"cartas?%s\"><image x = \"560\" y = \"500\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_pass.svg\" /></a>\n", script, path);
}

int pode_jogar(DATABASE data){
	int ind,n,v;
	int count=0;
	int r=0;
	int selec[3];
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
	if(count==data.nc&&data.nc!=0){
		for(i=0;i<data.nc;i++){
			n=selec[i]/13;
			v=selec[i]%13;
			max = maior_carta_jogada(data);
			if(v>max%13||(v==max%13&&n>max/13))
				r=1;
		}
	}
	//if(data.nc==0)
	//	r=1;
	return r;
}

//como é que vê que pode jogar quando os bots passaram todos??
void imprime_play (DATABASE data, char * path){
	int n,v;
	int ind;
	char script [52000];
	data.jogadas[0]=0;
	int count=0;
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
            	count ++;
        	}
    	}
    	data.selected = 0;
    	data.passar = 0;
    	data.play = 1;
    	DATA2STR(script, data);
    	printf("<a xlink:href = \"cartas?%s\"><image x = \"560\" y = \"460\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_play.svg\" /></a>\n", script, path);
    }
    else
    	printf("<image x = \"560\" y = \"460\" height = \"30\" width = \"90\" xlink:href = \"%s/botao_play_cinza.svg\" />\n", path);

}



void imprime_jogadas(DATABASE data, char * path){
	int x,y;
	int n,v;
	int jog;
	int ind;
	for(x=300,y=150,jog=2;jog>=0;jog-=2,y+=150){ //Pôr a ficar o jogador a somar?
        for(ind=0;ind<52;ind++){
            n = ind/13;
            v = ind%13;
            if(carta_existe(data.jogadas[jog],n,v)){
                imprime_carta_imagem(path,x,y,n,v);
                x +=20;
            }
        }
        x=300;
    }
    
    
    for(x=150,y=200,jog=3;jog>0;jog-=2,x+=350){//Pôr a ficar o jogador a somar?
        for(ind=0;ind<52;ind++){
            n = ind/13;
            v = ind%13;
            if(carta_existe(data.jogadas[jog],n,v)){
                imprime_carta_imagem(path,x,y,n,v);
                y +=20;
            }
        }
        y=200;
    }
}




void imprime_maos (DATABASE data, char * path){
	int n,v;
	int x,y;
	int jog;
	int ind;
	for(y = 430, jog = 0; jog < 3; jog+=2, y -= 420)
        for(x = 200, ind = 0; ind < 52; ind++){
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
                    imprime_carta_imagem(path,x,y,n,v);
                    x += 20;
                }
            }
        }
    for(x = 10, jog = 3; jog > 0; jog-=2, x += 650)//alterar para ser a somar o jog?
    	for(y = 100, ind = 0; ind < 52; ind++){
            n = ind/13;
            v = ind%13;
            if(carta_existe(data.mao[jog],n,v)==1){
                imprime_carta_imagem(path,x,y,n,v);
                y += 20;
            }
        }

}



//##############################Funções para imprimir-FIM##############################################


//##############################Funções bots#######################################################
DATABASE check_cartas(int n, int v, DATABASE data,int m){
	int count=0;
	int naipe[4] = {-1};
	int n1,v1;
	naipe[0]=n;
	int i = 1;
	count = data.nc - 1;
	for(n1=0;n1<4&&count>0;n1++)
		if(n1!=n&&carta_existe(data.mao[m],n1,v)){
			naipe[i]=n1;
			i++;
			count--;
		}
	if(count == 0)
		for(i=0;i<4;i++)
			if(naipe[i]!=-1){
				data.mao[m] = rem_carta(data.mao[m],naipe[i],v);
				data.jogadas[m] = add_carta(data.jogadas[m],naipe[i],v);
			}

	return data;
}

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

	printf("<svg height = \"800\" width = \"800\">\n");
    printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");
    if(data.inicio == 1){
    	jog = quem_comeca(data);
    	if (jog>0)
    		for(jog;jog<4;jog++){
    			data=joga_bots(data,jog);
    		}
    	data.inicio = 2;
    }
    if(data.play == 1)
    	for(jog=1;jog<4;jog++)
    		data = joga_bots(data,jog);

    imprime_maos(data,path);
    imprime_jogadas(data,path);
    imprime_play(data,path);
    imprime_passar(data,path);
}

//*****************************Função central- FIM################################

//pequenas coisas a corrigir
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

void check_finish(char * path, DATABASE data){
    int f=0,b;
    for(b=0;b<4;b++)
        if(data.mao[b]==0){
            f=1;
            break;
        }
    if(f==1){
        printf("Fim<br>");
    	printf("Pontuação:<br>Jogador - %d<br>Bot1 - %d<br>Bot2 - %d<br>Bot3 - %d",calcula_score(data.mao[0]),calcula_score(data.mao[1]),calcula_score(data.mao[2]),calcula_score(data.mao[3]));
    }
    else
        imprime(path,data);
}

void check_start(char * path, DATABASE data){
    int y, x,p,ind,n,v;
    printf("<svg height = \"800\" width = \"800\">\n");
    printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");
    if(data.inicio==0){
        for(y = 430, p = 0; p < 3; p+=2, y -= 420)
            for(x = 200, ind = 0; ind < 52; ind++){
                n = ind/13;
                v = ind%13;
                if(carta_existe(data.mao[p],n,v)==1){
                    imprime_carta_imagem(path,x,y,n,v);
                    x += 20;
                }
            }
        for(x = 10, p = 3; p > 0; p-=2, x += 650)
            for(y = 100, ind = 0; ind < 52; ind++){
                n = ind/13;
                v = ind%13;
                if(carta_existe(data.mao[p],n,v)==1){
                    imprime_carta_imagem(path,x,y,n,v);
                    y += 20;
                }
            }
        data.inicio=1;
        imprime_start(data,path);
        printf("</svg>\n");
    }
    else
        imprime(path,data);
    
}

void parse (char * query) {
    DATABASE data = {{0},0,{0},0,0,0,0};
    if(query!=NULL && strlen(query) != 0){ //n sei para q é preciso a primeira condição...
        data = STR2DATA(query);
        check_finish(BARALHO,data);
    }
    else{
        data = distribui(data); //é a primeira jogada, quem será que tem o precioso 3 de ouros?
        check_start(BARALHO, data);
    }
}


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