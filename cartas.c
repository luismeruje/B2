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

/**
	Estado inicial com todas as 52 cartas do baralho
	Cada carta é representada por um bit que está
	a 1 caso ela pertença à mão ou 0 caso contrário
*/
const long long int ESTADO_INICIAL = 0xfffffffffffff;


/** \brief Devolve o índice da carta

@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
@return		O índice correspondente à carta
*/
int indice(int naipe, int valor) {
	return naipe * 13 + valor;
}

/** \brief Adiciona uma carta ao estado

@param ESTADO	O estado atual
@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
@return		O novo estado
*/
long long int add_carta(long long int ESTADO, int naipe, int valor) {
	int idx = indice(naipe, valor);
	return ESTADO | ((long long int) 1 << idx);
}

/** \brief Remove uma carta do estado

@param ESTADO	O estado atual
@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
@return		O novo estado
*/
long long int rem_carta(long long int ESTADO, int naipe, int valor) {
	int idx = indice(naipe, valor);
	return ESTADO & ~((long long int) 1 << idx);
}

/** \brief Verifica se uma carta pertence ao estado

@param ESTADO	O estado atual
@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
@return		1 se a carta existe e 0 caso contrário
*/
int carta_existe(long long int ESTADO, int naipe, int valor) {
	int idx = indice(naipe, valor);
	return (ESTADO >> idx) & 1;
}

/** \brief Imprime o html correspondente a uma carta

@param path	o URL correspondente à pasta que contém todas as cartas
@param x A coordenada x da carta
@param y A coordenada y da carta
@param ESTADO	O estado atual
@param naipe	O naipe da carta (inteiro entre 0 e 3)
@param valor	O valor da carta (inteiro entre 0 e 12)
*/
void imprime_carta(char *path, int x, int y, long long int ESTADO, int naipe, int valor) {
	char *suit = NAIPES;
	char *rank = VALORES;
	char script[10240];
	sprintf(script, "%s?q=%lld", SCRIPT, rem_carta(ESTADO, naipe, valor));
	printf("<a xlink:href = \"%s\"><image x = \"%d\" y = \"%d\" height = \"110\" width = \"80\" xlink:href = \"%s/%c%c.svg\" /></a>\n", script, x, y, path, rank[valor], suit[naipe]);
}

void distribui (int jogador[4/*ndojog*/][14/*numerodacarta+1*/][2/*naipeouvalor*/]){ //exeção, jogador[x][13][0] tem o número de cartas já atribuidas ao jogador x
    int valor, naipe,jog;
    jogador[0][13][0] = 0;
    jogador[1][13][0] = 0;
    jogador[2][13][0] = 0;
    jogador[3][13][0] = 0;
    naipe = 0;
    while (naipe<4){
        valor = 0;
        while (valor<13){
            jog= rand() % 4;
            if(jogador[jog][13][0]<13){ //jasus, até mete as cartas certinhas no sítio, so proud, valor vale como valor da carta no loop e como numero da carta no array do jogador
                jogador[jog][jogador[jog][13][0]][0] = naipe; //usar antes indice da carta para poupar memória?
                jogador[jog][jogador[jog][13][0]][1] = valor; //pode calhar duas cartas de mm valor ao mm jogador dumbass
                jogador[jog][13][0]++;
                valor++;
            }
        }
        naipe++;
    }
}

/** \brief Imprime o estado

Esta função está a imprimir o estado em quatro colunas: uma para cada naipe
@param path	o URL correspondente à pasta que contém todas as cartas
@param ESTADO	O estado atual
*/
void imprime(char *path, long long int ESTADO) {
	int n, v;
	int x, y;
    int jogador[4][14][2];

	printf("<svg height = \"800\" width = \"800\">\n");
	printf("<rect x = \"0\" y = \"0\" height = \"800\" width = \"800\" style = \"fill:#007700\"/>\n");
    distribui(jogador); //TODO: está a baralhar sempre que clico numa carta em vez de a tirar, só fazer quando estado = 1111111111.. e verificar se existe.
    for(y = 10, n = 0; n < 2; n++, y += 420) {
        for(x = 100, v = 0; v < 13; v++){
            imprime_carta(path,x,y,ESTADO,jogador[n][v][0],jogador[n][v][1]);
            x+=20;
            }
    }
    for(x = 10, n = 2; n < 4; n++, x += 460) {
        for(y = 60, v = 0; v < 13; v++){
            imprime_carta(path,x,y,ESTADO, jogador[n][v][0],jogador[n][v][1]);
            y += 20;
            }
    }
	printf("</svg>\n");
}

/** \brief Trata os argumentos da CGI

Esta função recebe a query que é passada à cgi-bin e trata-a.
Neste momento, a query contém o estado que é um inteiro que representa um conjunto de cartas.
Cada carta corresponde a um bit que está a 1 se essa carta está no conjunto e a 0 caso contrário.
Caso não seja passado nada à cgi-bin, ela assume que todas as cartas estão presentes.
@param query A query que é passada à cgi-bin
 */
void parse(char *query) {
	long long int ESTADO;
	if(sscanf(query, "q=%lld", &ESTADO) == 1) {
		imprime(BARALHO, ESTADO);
	} else {
		imprime(BARALHO, ESTADO_INICIAL);
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
    srand(time(NULL));
	printf("<h1>Exemplo de utilização</h1>\n");

/*
 * Ler os valores passados à cgi que estão na variável ambiente e passá-los ao programa
 * O programa é executado de cada vez que se clica no ecrã? Se sim, como é que guarda o estado de umas vezes para as outras, no environment?
 * Como é que o input do utilizador é descodificado?
 */
	parse(getenv("QUERY_STRING"));

	printf("</body>\n");
	return 0;
}
