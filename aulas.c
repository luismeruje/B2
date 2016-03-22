#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
  

typedef long long int MAO;
struct estado {
  MAO mao[4];
  MAO highlight;
  int cartas[4];
  int play, pass, card;
};

typedef struct estado ESTADO;
char* estado2str(ESTADO e){
  static char str[10240];
  sprintf(str, "%lld_%lld_%lld_%lld_%lld_%d_%d_%d_%d_%d_%d_%d", e.mao[0], e.mao[1], e.mao[2], e.mao[3], e.highlight, e.cartas[0], e.cartas[1], e.cartas[2], e.cartas[3], e.play, e.pass, e.card);

  return str;
}

ESTADO str2estado(char* str){
  ESTADO e;
  sscanf(str, "%lld_%lld_%lld_%lld_%lld_%d_%d_%d_%d_%d_%d_%d", &e.mao[0], &e.mao[1], &e.mao[2], &e.mao[3], &e.highlight, &e.cartas[0], &e.cartas[1], &e.cartas[2], &e.cartas[3], &e.play, &e.pass, &e.card);  

  return e;
}

int main(){
  ESTADO e, e1;
  char str[10240];
  int i;
  for(i=0;i<4;i++){
    e.mao[i]=0;
    e.cartas[i]=0;
  }
  e.highlight=0;
  e.play=e.pass=e.card=0;
  printf("%s\n", estado2str(e));

  scanf("%s", str);
  e=str2estado(str);
  e1 = e;
  e1.play=1;
  printf("<a xlink:href=\"localhost/cgi-bin/cartas?%s\">asdf</a>\n", estado2str(e1));  
}
