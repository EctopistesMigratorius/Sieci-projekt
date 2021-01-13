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
  int gniazdo, link1, link2;
  int msgx, msgo;
  int msgs_size, msgptr;
  int sendbytes;
  struct sockaddr_in adr, nadawca1, nadawca2;

  char *msgscated;
  char **msgsx;
  char **msgso;
  msgx=1;
  msgo=1;
  msgsx = malloc(msgx * sizeof(char*));
  msgso = malloc(msgo * sizeof(char*));

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
  if((link1 = accept(gniazdo, (struct sockaddr*) &nadawca1, &dl))>0){
    printf("Pierwszy gracz połączony\n");
  }
  if((link2 = accept(gniazdo, (struct sockaddr*) &nadawca2, &dl))>0){
    printf("Drugi gracz połączony\n");
  }

  memset(bufor1, 0, 1024);
  memset(bufor2, 0, 1024);
  recv(link1, bufor1, 1024, 0);
  /*recv(gniazdo3, bufor2, 1024, 0);*/
  printf("%s\n", bufor1);

  active_player = random()%2;
  if(active_player==0){
    send(link1, "GIVE_O", 14, 0);
    /*send(gniazdo3, "GIVE_X", 15, 0);*/
  }
  else{
    /*send(gniazdo3, "GIVE_O", 14, 0);*/
    send(link1, "GIVE_X", 15, 0);
  }


  /*if(fork() == 0){*/
  while(1){
    memset(bufor1, 0, 1024);
    memset(bufor2, 0, 1024);
    recv(link1, bufor1, 1024, 0);
    /*recv(link2, bufor2, 1024, 0);*/

    if(strcmp(bufor1, "SENDMSG_X") == 0){
      memset(bufor1, 0, 1024);
      recv(link1, bufor1, 1024, 0);

      msgsx[msgx-1] = malloc((strlen(bufor1)+3) * sizeof(char));
      msgsx[msgx-1][0]='X';
      msgsx[msgx-1][1]=':';
      strcat(msgsx[msgx-1], bufor1);
      msgsx[msgx-1][strlen(bufor1)+2] = '\0';
      msgx = msgx + 1;
      msgsx = realloc(msgsx, msgx * sizeof(char));

      msgso[msgo-1] = malloc((strlen(bufor1)+3) * sizeof(char));
      msgso[msgo-1][0]='X';
      msgso[msgo-1][1]=':';
      strcat(msgso[msgo-1], bufor1);
      msgso[msgo-1][strlen(bufor1)+2] = '\0';
      msgo = msgo + 1;
      msgso = realloc(msgso, msgo * sizeof(char));
    }

    if(strcmp(bufor1, "SENDMSG_O") == 0){
      memset(bufor1, 0, 1024);
      recv(link1, bufor1, 1024, 0);

      msgsx[msgx-1] = malloc((strlen(bufor1)+3) * sizeof(char));
      msgsx[msgx-1][0]='O';
      msgsx[msgx-1][1]=':';
      strcat(msgsx[msgx-1], bufor1);
      msgsx[msgx-1][strlen(bufor1)+2] = '\0';
      msgx = msgx + 1;
      msgsx = realloc(msgsx, msgx * sizeof(char));

      msgso[msgo-1] = malloc((strlen(bufor1)+3) * sizeof(char));
      msgso[msgo-1][0]='O';
      msgso[msgo-1][1]=':';
      strcat(msgso[msgo-1], bufor1);
      msgso[msgo-1][strlen(bufor1)+2] = '\0';
      msgo = msgo + 1;
      msgso = realloc(msgso, msgo * sizeof(char));
    }

    if(strcmp(bufor1, "GETMSG_O") == 0){
      msgs_size = 0;
      msgptr = 0;
      sendbytes = 0;
      for (int i = 0; i < msgo-1; i++){
        msgs_size = msgs_size + strlen(msgso[i]);
      }
      msgscated = malloc(msgs_size * sizeof(char));
      for (int i = 0; i < msgo-1; i++){
        memcpy(&msgscated[msgptr], msgso[i], strlen(msgso[i]));
        msgptr = msgptr + strlen(msgso[i]);
      }
      memset(bufor1, 0, 1024);
      sendbytes = strlen(msgscated);
      msgptr = 0;
      while(sendbytes>48){
        memcpy(bufor1, &msgscated[0]+msgptr, 48);
        send(link1, bufor1, 48, 0);
        msgptr = msgptr + 48;
        sendbytes = sendbytes - 48;
      }
      memcpy(bufor1, &msgscated[0]+msgptr, sendbytes);
      send(link1, bufor1, sendbytes, 0);
      send(link1, '\0', 1, 0);
      msgo = 1;
      msgso = realloc(msgso, msgo * sizeof(char));
    }

    if(strcmp(bufor1, "GETMSG_X") == 0){
      msgs_size = 0;
      msgptr = 0;
      sendbytes = 0;
      for (int i = 0; i < msgx-1; i++){
        msgs_size = msgs_size + strlen(msgsx[i]);
      }
      msgscated = malloc(msgs_size * sizeof(char));
      for (int i = 0; i < msgx-1; i++){
        memcpy(&msgscated[msgptr], msgsx[i], strlen(msgsx[i]));
        msgptr = msgptr + strlen(msgsx[i]);
      }
      memset(bufor1, 0, 1024);
      sendbytes = strlen(msgscated);
      msgptr = 0;
      while(sendbytes>48){
        memcpy(bufor1, &msgscated[0]+msgptr, 48);
        send(link1, bufor1, 48, 0);
        msgptr = msgptr + 48;
        sendbytes = sendbytes - 48;
      }
      memcpy(bufor1, &msgscated[0]+msgptr, sendbytes);
      send(link1, bufor1, sendbytes, 0);
      send(link1, '\0', 1, 0);
      msgx = 1;
      msgso = realloc(msgsx, msgx * sizeof(char));
    }
  }
  close(gniazdo);
  return 0;
}
