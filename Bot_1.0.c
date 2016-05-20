//
//  Bot_1.0.c
//  
//
//  Created by Luís Manuel Meruje Ferreira on 17/05/16.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef long long int MAO;

typedef struct database{
    MAO mao[4]; //n sei se é preciso
    int nc;
    MAO jogada;
} DATABASE;

typedef struct searchtree{
    long long int estado;
    long int t;
    float r; //reward
    int play;
    struct searchtree * nextN[4][40]; //TODO: tentar diminuir o 40
    struct searchtree * prev;
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

long long int straightpos (MAO mao) {
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

long long int flushpos (MAO mao) {
    MAO max = 0;
    int i;
    int naipe[4] = {0};
    separa_nap(mao, naipe);
    for (i=3; i>=0; i--) {
        if(naipe[i]>4) break;
    }
    if(i != -1) {
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
    int i, p=0;
    
    separa_val(mao, rank);
    for (i=12; i>=0; i--)
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
                c++;
            }
            n++;
        }
    }
    return max;
}

long long int fourofakindpos (MAO mao) {
    MAO max = 0;
    int i;
    int rank[13] = {0};
    separa_val(mao, rank);
    for(i=12; i>=0; i--)
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

long long int straightflushpos (MAO mao) {
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
    for(i = 0; i < 100; i++)
        for(nc = 0; nc < 4; nc++)
        	tree -> nextN[nc][i] = NULL;
    tree->prev = NULL;
    return tree;
}

void addNodo(MAO mao,MCtree tree, MCtree previous){
    int i,nc;
	tree = malloc(sizeof(searchTree));
    tree -> t = 0;
    tree -> r = 0;
    tree -> play = 0;
    tree -> estado = mao;
    for(i = 0; i < 40; i++)
        for(nc = 0; nc < 4; nc++)
            tree -> nextN[nc][i] = NULL;
    tree->prev = previous;
}

void rewardF(MCtree temp, DATABASE * simulacao){
    int i, n, v;
    float count = 0.0;
    for(i = 0; i < 52; i++){
        n = i / 13;
        v = i % 13;
        if(carta_existe(simulacao -> mao[0],n,v)) //TODO: ter em conta as cartas nas mãos dos outros?
            count++;
    }
    if(count <= 9)
        count = 1.0 - (count * 0.03);
    else if(count == 13)
        count = 1.0 - (count * 0.06);
    else
        count = 0.0;
    while(temp != NULL){
        temp->r += count;
        temp->t++;
        temp = temp->prev;
    }
}


//tem uma cópia do data própria para poder manipulá-la
//UCT : UCT_value[i] = (((tree->nextN[i])->r) / ((tree->nextN[i])->t)) + 1.4142136 * (sqrt(log(tree->t) / ((tree->nextN[i])->r))); //check poss. erros
MCtree treePolicy(MCtree tree, DATABASE * simulacao){
    int i, jog[4] = {0}, UCT_value[100] = {0};//posso meter else em baixo, que mete a zero o que n for play = 1
    int max = 0, nc;
    if(tree->nextN[0] == NULL){
        add_jogadas(tree->estado, jog); //adiciona jogadas ao array nextN e devolve o número de jogadas para cada nc.
        if(simulacao -> nc == 0){
        	nc = rand() % 4;
            i = rand() % jog[nc];
            while(tree->nextN[nc][i]==NULL){
            	nc = rand() % 4;
                i = rand() % jog[nc];
            }
            return tree->nextN[nc][i];
        }
        i = rand() % jog[simulacao->nc];
        return tree -> nextN[simulacao -> nc][i];
    }
    else{
        for(i = 0; tree->nextN[simulacao -> nc][i]!= NULL && i < 100; i++){
            if((tree->nextN[simulacao -> nc][i]) -> t == 0) //TODO: se mais do que um for zero, fazer rand, para este n deve ser preciso mas para se o UCT for igaul em baixo é melhor fazer
                return tree->nextN[simulacao -> nc][i];
            UCT_value[i] = (((tree->nextN[nc][i])->r) / ((tree->nextN[nc][i])->t)) + 1.4142136 * (sqrt(log(tree->t) / ((tree->nextN[nc][i])->r)));
        	if(UCT_value[i] > UCT_value[max])
                max = i;
        }
        return tree->nextN[simulacao -> nc][max];
    }
}

//TODO: jogadas_possiveis, preenche o array com as jogadas e conta as jogadas para cada nc
int defaultPolicy(MCtree node, DATABASE * simulacao){
    int jogo = 0, jog[4] = {0}, flag = 0, j = 0, passar; //TODO: implementar o passar para ver se se mete o nc = 0 e testar se estamos num nodo que é para simular até ao fim ou só um passo
    MAO jogadas[5][40] = {-1};//WARNING: o -1 pode dar problema com o operador binário, meter a 0 primeiro a jogada na jogadas_possiveis//TODO: tentar baixar o 40
    int i, nc;
    while(jogo == 0){
    	if(simulacao->nc == 0){
        	jogadas_possiveis(node->estado, simulacao, jog, j);
        	nc = rand() % 4;
        	i = rand() % jog[nc];
        	while(jogadas[nc][i] <= 0){ //WARNING: e se for passar?
           	 	nc = rand() % 4;
            	i = rand() % jog[nc];
            }
            simulacao->mao[j] = simulacao->mao[j]  ^ jogadas[nc][i];
            if(simulacao->mao[j] == 0)
                jogo = 1;
            else{
                j = (j + 1) % 4;
                simulacao->jogada = jogadas[nc][i];
                simulacao->nc = nc;
            }
        }
        else{
        
        }
    }
        
    return flag;
}


int main(){
    char input[100];
    int flag = 0;
    MCtree temp;
    DATABASE data; //data precisa de ser controlado a partir do input
    DATABASE simulacao = data;
    //só é para executar isto uma vez
    MCtree tree; //para usar com o create tree
    
    scanf("%s", &input);
    while(strcmp(input, "FIM") == 0){
        //Pôr outros casos, se der outras coisas mudar o data, senão fazer o que está abaixo, se for o que dá a mão, executar create tree
        while(flag == 0){
        	temp = treePolicy(tree, &simulacao);
    		flag = defaultPolicy(temp, &simulacao);
        }
        flag = 0;
        simulacao = data;
        scanf("%s", &input);
    }
    return 0;
}
