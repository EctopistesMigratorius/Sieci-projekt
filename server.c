#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

/*
0|1|2
3|4|5
6|7|8

serwer jako stan planszy przyjmuje string z 9 znakow:
'0'-puste pole
'1'-kolko
'2'-krzyzyk
*/

int gameCheck (char tab[]){
  if((tab[0]==tab[1] && tab[1]==tab[2] && tab[0]!='0') || (tab[0]==tab[4] && tab[4]==tab[8] && tab[0]!='0') || (tab[0]==tab[3] && tab[3]==tab[6] && tab[0]!='0') || (tab[3]==tab[4] && tab[4]==tab[5] && tab[3]!='0') || (tab[1]==tab[4] && tab[4]==tab[7] && tab[1]!='0') || (tab[2]==tab[5] && tab[5]==tab[8] && tab[2]!='0') || (tab[6]==tab[7] && tab[7]==tab[8] && tab[6]!='0') || (tab[2]==tab[4] && tab[4]==tab[6] && tab[2]!='0')){
    return 1;
  }
  else{
    return 0;
  }
}

int main(void) {
  unsigned int port;
  char bufor1[1024], bufor2[1024];
  int gniazdo, gniazdo2, gniazdo3;
  struct sockaddr_in adr, nadawca1, nadawca2;
  socklen_t dl = sizeof(struct sockaddr_in);

  struct timeval tm;
  int active_player;
  gettimeofday(&tm, NULL);
  srandom(tm.tv_sec + tm.tv_usec * 1000000ul);

  printf("Na ktorym porcie mam sluchac? : ");
  scanf("%u", &port);
  gniazdo = socket(PF_INET, SOCK_STREAM, 0);
  adr.sin_family = AF_INET;
  adr.sin_port = htons(port);
  adr.sin_addr.s_addr = INADDR_ANY;
  if (bind(gniazdo, (struct sockaddr*) &adr,
  sizeof(adr)) < 0) {
    printf("Bind nie powiodl sie.\n");
    return 1;
  }
  if (listen(gniazdo, 10) < 0) {
    printf("Listen nie powiodl sie.\n");
    return 1;
  }
  printf("Czekam na polaczenie ...\n");
  if((gniazdo2 = accept(gniazdo, (struct sockaddr*) &nadawca1, &dl))>0){
    printf("Pierwszy gracz połączony\n");
  }
  if((gniazdo3 = accept(gniazdo, (struct sockaddr*) &nadawca2, &dl))>0){
    printf("Drugi gracz połączony\n");
  }
  active_player = random()%2;
  if(active_player==0){
    send(gniazdo2, "You go first!", 14, 0);
    send(gniazdo3, "You go second!", 15, 0);
  }
  else{
    send(gniazdo3, "You go first!", 14, 0);
    send(gniazdo2, "You go second!", 15, 0);
  }
  if(fork() == 0){
    while(1){
      memset(bufor1, 0, 1024);
      recv(gniazdo2, bufor1, 1024, 0);
      if(strcmp(bufor1, "game")){
        memset(bufor1, 0, 1024);
        recv(gniazdo2, bufor1, 1024, 0);
        if(gameCheck(bufor1)){
          send(gniazdo3, "end", 4, 0);
          send(gniazdo3, "First player have won!", 23, 0);
          send(gniazdo2, "end", 4, 0);
          send(gniazdo2, "Player first have won!", 23, 0);
        }
        else{
          send(gniazdo3, "game", 5, 0);
          send(gniazdo3, bufor1, 1024, 0);
        }
      }
      if(strcmp(bufor1, "msg")){
        memset(bufor1, 0, 1024);
        recv(gniazdo2, bufor1, 1024, 0);
        send(gniazdo3, "msg", 4, 0);
        send(gniazdo3, bufor1, 1024, 0);
      }
      if(strcmp(bufor1, "img")){
        memset(bufor1, 0, 1024);
        recv(gniazdo2, bufor1, 1024, 0);
        /* tu jeszcze ogarnę*/
      }
    }
  }
  else{
    memset(bufor2, 0, 1024);
    recv(gniazdo3, bufor2, 1024, 0);
    if(strcmp(bufor2, "game")){
      memset(bufor2, 0, 1024);
      recv(gniazdo3, bufor2, 1024, 0);
      if(gameCheck(bufor2)){
        send(gniazdo2, "end", 4, 0);
        send(gniazdo2, "Second player have won!", 24, 0);
        send(gniazdo3, "end", 4, 0);
        send(gniazdo3, "Second player have won!", 24, 0);
      }
      else{
        send(gniazdo2, "game", 5, 0);
        send(gniazdo2, bufor2, 1024, 0);
      }
    }
    if(strcmp(bufor2, "msg")){
      memset(bufor2, 0, 1024);
      recv(gniazdo3, bufor2, 1024, 0);
      send(gniazdo2, "msg", 4, 0);
      send(gniazdo2, bufor2, 1024, 0);
    }
    if(strcmp(bufor2, "img")){
      memset(bufor2, 0, 1024);
      recv(gniazdo3, bufor2, 1024, 0);
      /* tu jeszcze ogarnę*/
    }
  }
  close(gniazdo);
  return 0;
}
