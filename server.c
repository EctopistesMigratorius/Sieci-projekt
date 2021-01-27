#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/mman.h>

int boardFull(int board[]){
  for (int i = 0; i < 9; i++){
    if (board[i] == 0){
      return 0;
    }
  }
  return 1;
}

int compareArray(int a, int b, int c, int board[]){
  if(board[a] == board[b] && board[b] == board[c] && board [a] != 0){
    return 1;
  }
  else{
    return 0;
  }
}

int gameCheck(int board[]){
  if(compareArray(0,1,2,board) || compareArray(3,4,5,board) || compareArray(6,7,8,board) || compareArray(0,3,6,board) || compareArray(1,4,7,board) || compareArray(2,5,8,board) || compareArray(0,4,8,board) || compareArray(2,4,6,board)){
    return 2;
  }
  else{
    if(boardFull(board)){
      return 1;
    }
    else{
      return 0;
    }
  }
}

void* create_shared_memory(size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;

  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_SHARED | MAP_ANONYMOUS;

  // The remaining parameters to `mmap()` are not important for this use case,
  // but the manpage for `mmap` explains their purpose.
  return mmap(NULL, size, protection, visibility, -1, 0);
}

int main(void) {
  unsigned int port1, port2;
  char bufor1[1025], bufor2[1025];
  char dest[1024];
  char fdest[1024], fname[1024];
  int socket1, socket2, link1, link2;
  int msgs_size, msgptr;
  int dataptr;
  int sendbytes;
  size_t *encodesize;
  int bytes, newbytes, filesize;
  struct sockaddr_in adr1, adr2, nadawca1, nadawca2;
  FILE *f;
  int gameboard[9];
  char gamesend[19];
  struct timeval tm;
  int active_player;
  char *msgscated;
  char *file;
  char *data;
  char msgsx [5000];
  char msgso [5000];
  void* shmem_msgx = create_shared_memory(20);
  void* shmem_msgo = create_shared_memory(20);
  void* shmem_gamesend = create_shared_memory(20);
  void* shmem_gameboard = create_shared_memory(10);
  void* shmem_msgsx = create_shared_memory(5000);
  void* shmem_msgso = create_shared_memory(5000);

  gettimeofday(&tm, NULL);
  srandom(tm.tv_sec + tm.tv_usec * 1000000ul);

  strcpy(dest, "/home/pigeon/files/");

  memset(gamesend, ' ', 19);
  for (int i = 0; i<9; i++){
    gameboard[i]=0;
    gamesend[2*i]='N';
  }
  active_player = random()%2;
  if(active_player==0){
    gamesend[18] = 'O';
  }
  else{
    gamesend[18] = 'X';
  }

  memset(msgsx, 0, 5000);
  memset(msgso, 0, 5000);

  memcpy(shmem_gamesend, gamesend, sizeof(gamesend));
  memcpy(shmem_gameboard, gameboard, sizeof(gameboard));
  memcpy(shmem_msgsx, msgsx, sizeof(msgsx));
  memcpy(shmem_msgso, msgso, sizeof(msgso));

  socklen_t dl = sizeof(struct sockaddr_in);

  /*printf("Na ktorym porcie mam sluchac gracza pierwszego? : ");
  scanf("%u", &port1);
  printf("Na ktorym porcie mam sluchac gracza drugiego? : ");
  scanf("%u", &port2);*/
  port1=3060;
  port2=3061;
  socket1 = socket(PF_INET, SOCK_STREAM, 0);
  socket2 = socket(PF_INET, SOCK_STREAM, 0);
  adr1.sin_family = AF_INET;
  adr1.sin_port = htons(port1);
  adr1.sin_addr.s_addr = INADDR_ANY;
  adr2.sin_family = AF_INET;
  adr2.sin_port = htons(port2);
  adr2.sin_addr.s_addr = INADDR_ANY;

  if (bind(socket1, (struct sockaddr*) &adr1,
  sizeof(adr2)) < 0) {
    printf("Bind #1 nie powiodl sie.\n");
    return 1;
  }
  if (bind(socket2, (struct sockaddr*) &adr2,
  sizeof(adr2)) < 0) {
    printf("Bind #2 nie powiodl sie.\n");
    return 1;
  }

  if (listen(socket1, 10) < 0) {
    printf("Listen #1 nie powiodl sie.\n");
    return 1;
  }

  if (listen(socket2, 10) < 0) {
    printf("Listen #2 nie powiodl sie.\n");
    return 1;
  }

  printf("Czekam na polaczenie ...\n");

  if((link1 = accept(socket1, (struct sockaddr*) &nadawca1, &dl))>0){
    printf("Pierwszy gracz połączony\n");
  }
  if((link2 = accept(socket2, (struct sockaddr*) &nadawca2, &dl))>0){
    printf("Drugi gracz połączony\n");
  }

  memset(bufor1, 0, 1025);
  memset(bufor2, 0, 1025);
  recv(link1, bufor1, 1024, 0);
  recv(link2, bufor2, 1024, 0);
  printf("Pierwszy: %s\n", bufor1);
  printf("Drugi: %s\n", bufor2);
  send(link1, "GIVE_O", 6, 0);
  send(link2, "GIVE_X", 6, 0);

  if(fork()==0){
    while(1){
      memset(bufor1, 0, 1024);
      if(recv(link1, bufor1, 1024, 0) > 0){
        printf("Pierwszy: %s\n", bufor1);
      }

      if(strcmp(bufor1, "GAME_O") == 0){
        send(link1, "OK", 2, 0);
        memset(bufor1, 0, 1024);
        recv(link1, bufor1, 1024, 0);
        memcpy(gamesend, shmem_gamesend, sizeof(gamesend));
        memcpy(gameboard, shmem_gameboard, sizeof(gameboard));
        gameboard[atoi(bufor1)] = 1;
        gamesend[atoi(bufor1)*2] = 'O';
        gamesend[18] = 'X';
        printf("Changed: %s\n", gamesend);
        if(gameCheck(gameboard)==2){
          memcpy(msgsx, shmem_msgsx, sizeof(msgsx));
          memcpy(msgso, shmem_msgso, sizeof(msgso));

          char *msgx = malloc((strlen("O wygrywa")+4)*sizeof(char));
          memset(msgx, 0, strlen(bufor1)+4);
          msgx[0]='G';
          msgx[1]=':';
          msgx[2]=' ';
          strcat(msgx, "O wygrywa");
          msgx[strlen("O wygrywa")+3]='\0';
          strcat(msgsx, msgx);

          char *msgo = malloc((strlen("O wygrywa")+4)*sizeof(char));
          memset(msgo, 0, strlen("O wygrywa")+4);
          msgo[0]='G';
          msgo[1]=':';
          msgo[2]=' ';
          strcat(msgo, "O wygrywa");
          msgo[strlen("O wygrywa")+3]='\0';
          strcat(msgso, msgo);

          memcpy(shmem_msgsx, msgsx, sizeof(msgsx));
          memcpy(shmem_msgso, msgso, sizeof(msgso));

          free(msgx);
          free(msgo);
        }
        else{
          if (gameCheck(gameboard)==1){
            memcpy(msgsx, shmem_msgsx, sizeof(msgsx));
            memcpy(msgso, shmem_msgso, sizeof(msgso));

            char *msgx = malloc((strlen("Remis")+4)*sizeof(char));
            memset(msgx, 0, strlen(bufor1)+4);
            msgx[0]='G';
            msgx[1]=':';
            msgx[2]=' ';
            strcat(msgx, "Remis");
            msgx[strlen("Remis")+3]='\0';
            strcat(msgsx, msgx);

            char *msgo = malloc((strlen("Remis")+4)*sizeof(char));
            memset(msgo, 0, strlen("Remis")+4);
            msgo[0]='G';
            msgo[1]=':';
            msgo[2]=' ';
            strcat(msgo, "Remis");
            msgo[strlen("Remis")+3]='\0';
            strcat(msgso, msgo);

            memcpy(shmem_msgsx, msgsx, sizeof(msgsx));
            memcpy(shmem_msgso, msgso, sizeof(msgso));

            free(msgx);
            free(msgo);
          }
        }
        memcpy(shmem_gamesend, gamesend, sizeof(gamesend));
        memcpy(shmem_gameboard, gameboard, sizeof(gameboard));
        send(link1, "OK", 2, 0);
      }

      if(strcmp(bufor1, "BOARD") == 0){
        memcpy(gamesend, shmem_gamesend, sizeof(gamesend));
        send(link1, gamesend, 19, 0);
      }

      if(strcmp(bufor1, "SENDMSG_O") == 0){
        send(link1, "OK", 2, 0);
        memset(bufor1, 0, 1024);
        recv(link1, bufor1, 1023, 0);
        printf("SENDMSG_O: %s\n", bufor1);
        send(link1, "OK", 2, 0);
        memcpy(msgsx, shmem_msgsx, sizeof(msgsx));
        memcpy(msgso, shmem_msgso, sizeof(msgso));

        char *msgx = malloc((strlen(bufor1)+4)*sizeof(char));
        memset(msgx, 0, strlen(bufor1)+4);
        msgx[0]='O';
        msgx[1]=':';
        msgx[2]=' ';
        strcat(msgx, bufor1);
        msgx[strlen(bufor1)+3]='\0';
        strcat(msgsx, msgx);

        char *msgo = malloc((strlen(bufor1)+4)*sizeof(char));
        memset(msgo, 0, strlen(bufor1)+4);
        msgo[0]='O';
        msgo[1]=':';
        msgo[2]=' ';
        strcat(msgo, bufor1);
        msgo[strlen(bufor1)+3]='\0';
        strcat(msgso, msgo);

        memcpy(shmem_msgsx, msgsx, sizeof(msgsx));
        memcpy(shmem_msgso, msgso, sizeof(msgso));

        free(msgx);
        free(msgo);
      }

      if(strcmp(bufor1, "GETMSG_O") == 0){
        msgs_size = 0;
        msgptr = 0;
        sendbytes = 0;
        memcpy(msgso, shmem_msgso, sizeof(msgso));
        memset(bufor1, 0, 1025);
        sendbytes = strlen(msgso);
        msgptr = 0;
        printf("%d %s\n", sendbytes, msgso);
        while(sendbytes>1024){
          memcpy(bufor1, &msgso[0]+msgptr, 1024);
          printf("%s\n", bufor1);
          send(link1, bufor1, 1024, 0);
          msgptr = msgptr + 1024;
          sendbytes = sendbytes - 1024;
        }
        memcpy(bufor1, &msgso[0]+msgptr, sendbytes);
        if (sendbytes==0){
          send(link1, "OK", 2, 0);
        }
        else{
          send(link1, bufor1, sendbytes, 0);
        }
        if(sendbytes == 1024){
          send(link1, "OK", 2, 0);
        }
        memset(msgso, 0, 5000);
        memcpy(shmem_msgso, msgso, sizeof(msgso));
      }

      if(strcmp(bufor1, "DISCONNECT") == 0){
        send(link1, "OK", 2, 0);
        close(link1);
        close(socket1);
        break;
      }
      
      if(strcmp(bufor1, "SEND_FILE") == 0){
        bytes = 0;
        newbytes = 0;
        send(link1, "OK", 2, 0);
        memset(bufor1, 0, 1024);
        recv(link1, bufor1, 1023, 0);
        strcpy(fname, bufor1);
        printf("Nazwa Pliku: %s\n", bufor1);
        send(link1, "OK", 2, 0);
        memset(bufor1, 0, 1024);
        recv(link1, bufor1, 1023, 0);
        send(link1, "OK", 2, 0);
        filesize = atoi(bufor1);
        printf("Wielkość Pliku: %d\n", filesize);
        strcpy(fdest, dest);
        strcat(fdest, fname);
        f = fopen(fdest, "wb+");
        while(bytes < filesize){
          memset(bufor1, 0, 1024);
          newbytes = recv(link1, bufor1, 1023, 0);
          fwrite(bufor1, 1, strlen(bufor1), f);
          bytes += newbytes;
        }
        send(link1, "OK", 2, 0);
        fclose(f);
      }

      if(strcmp(bufor1, "GET_FILE") == 0){
        filesize = 0;
        send(link1, "OK", 2, 0);
        memset(bufor1, 0, 1024);
        recv(link1, bufor1, 1023, 0);
        strcpy(fname, bufor1);
        printf("Nazwa Pliku: %s\n", fname);
        memset(bufor1, 0, 1024);
        strcpy(fdest, dest);
        strcat(fdest, fname);
        memset(fname, 0, strlen(fname));
        if( access( fdest, F_OK ) != 0 ) {
          send(link2, "OK", 2, 0);
        }
        else{
          f = fopen(fdest, "r+");
          fseek(f, 0, SEEK_END);
          filesize = ftell(f);
          fseek(f, 0, SEEK_SET);
          data = malloc(1025 * sizeof(char));
          dataptr = 0;
          sendbytes = 0;
          sendbytes = filesize;
          while(sendbytes>1024){
              memset(data, 0, 1025);
              fread(data, 1024, 1, f);
              send(link1, data, 1024, 0);
              sendbytes = sendbytes - 1024;
          }
          if (sendbytes==0){
            send(link1, "OK", 2, 0);
          }
          else{
            memset(data, 0, 1025);
            fread(data, sendbytes, 1, f);
            send(link1, data, sendbytes, 0);
          }
          if(sendbytes == 1024){
            send(link1, "OK", 2, 0);
          }
          free(data);
          fclose(f);
        }
      }
    }
  }
  else{
    while(1){
      memset(bufor2, 0, 1024);
      if(recv(link2, bufor2, 1024, 0) > 0){
        printf("Drugi: %s\n", bufor2);
      }

      if(strcmp(bufor2, "GAME_X") == 0){
        send(link2, "OK", 2, 0);
        memset(bufor2, 0, 1024);
        recv(link2, bufor2, 1024, 0);
        memcpy(gamesend, shmem_gamesend, sizeof(gamesend));
        memcpy(gameboard, shmem_gameboard, sizeof(gameboard));
        gameboard[atoi(bufor2)] = 2;
        gamesend[atoi(bufor2)*2] = 'X';
        gamesend[18] = 'O';
        printf("Changed: %s\n", gamesend);
        if(gameCheck(gameboard)==2){
          memcpy(msgsx, shmem_msgsx, sizeof(msgsx));
          memcpy(msgso, shmem_msgso, sizeof(msgso));

          char *msgx = malloc((strlen("X wygrywa")+4)*sizeof(char));
          memset(msgx, 0, strlen(bufor1)+4);
          msgx[0]='G';
          msgx[1]=':';
          msgx[2]=' ';
          strcat(msgx, "X wygrywa");
          msgx[strlen("X wygrywa")+3]='\0';
          strcat(msgsx, msgx);

          char *msgo = malloc((strlen("X wygrywa")+4)*sizeof(char));
          memset(msgo, 0, strlen("X wygrywa")+4);
          msgo[0]='G';
          msgo[1]=':';
          msgo[2]=' ';
          strcat(msgo, "X wygrywa");
          msgo[strlen("X wygrywa")+3]='\0';
          strcat(msgso, msgo);

          memcpy(shmem_msgsx, msgsx, sizeof(msgsx));
          memcpy(shmem_msgso, msgso, sizeof(msgso));

          free(msgx);
          free(msgo);
        }
        else{
          if (gameCheck(gameboard)==1){
            memcpy(msgsx, shmem_msgsx, sizeof(msgsx));
            memcpy(msgso, shmem_msgso, sizeof(msgso));

            char *msgx = malloc((strlen("Remis")+4)*sizeof(char));
            memset(msgx, 0, strlen(bufor1)+4);
            msgx[0]='G';
            msgx[1]=':';
            msgx[2]=' ';
            strcat(msgx, "Remis");
            msgx[strlen("Remis")+3]='\0';
            strcat(msgsx, msgx);

            char *msgo = malloc((strlen("Remis")+4)*sizeof(char));
            memset(msgo, 0, strlen("Remis")+4);
            msgo[0]='G';
            msgo[1]=':';
            msgo[2]=' ';
            strcat(msgo, "Remis");
            msgo[strlen("Remis")+3]='\0';
            strcat(msgso, msgo);

            memcpy(shmem_msgsx, msgsx, sizeof(msgsx));
            memcpy(shmem_msgso, msgso, sizeof(msgso));

            free(msgx);
            free(msgo);
          }
        }
        memcpy(shmem_gamesend, gamesend, sizeof(gamesend));
        memcpy(shmem_gameboard, gameboard, sizeof(gameboard));
        send(link2, "OK", 2, 0);
      }

      if(strcmp(bufor2, "BOARD") == 0){
        memcpy(gamesend, shmem_gamesend, sizeof(gamesend));
        send(link2, gamesend, 19, 0);
      }

      if(strcmp(bufor2, "SENDMSG_X") == 0){
        send(link2, "OK", 2, 0);
        memset(bufor2, 0, 1024);
        recv(link2, bufor2, 1023, 0);
        send(link2, "OK", 2, 0);
        printf("SENDMSG_X: %s\n", bufor2);
        memcpy(msgsx, shmem_msgsx, sizeof(msgsx));
        memcpy(msgso, shmem_msgso, sizeof(msgso));

        char *msgx = malloc((strlen(bufor2)+4)*sizeof(char));
        memset(msgx, 0, strlen(bufor2)+4);
        msgx[0]='X';
        msgx[1]=':';
        msgx[2]=' ';
        strcat(msgx, bufor2);
        msgx[strlen(bufor2)+3]='\0';
        strcat(msgsx, msgx);

        char *msgo = malloc((strlen(bufor2)+4)*sizeof(char));
        memset(msgo, 0, strlen(bufor2)+4);
        msgo[0]='X';
        msgo[1]=':';
        msgo[2]=' ';
        strcat(msgo, bufor2);
        msgo[strlen(bufor2)+3]='\0';
        strcat(msgso, msgo);

        memcpy(shmem_msgsx, msgsx, sizeof(msgsx));
        memcpy(shmem_msgso, msgso, sizeof(msgso));
      }

      if(strcmp(bufor2, "GETMSG_X") == 0){
        msgs_size = 0;
        msgptr = 0;
        sendbytes = 0;
        memcpy(msgsx, shmem_msgsx, sizeof(msgsx));
        memset(bufor2, 0, 1025);
        sendbytes = strlen(msgsx);
        printf("%d %s\n", sendbytes, msgsx);
        while(sendbytes>1024){
          memcpy(bufor2, &msgsx[0]+msgptr, 1024);
          printf("%s\n", bufor2);
          send(link2, bufor2, 1024, 0);
          msgptr = msgptr + 1024;
          sendbytes = sendbytes - 1024;
        }
        memcpy(bufor2, &msgsx[0]+msgptr, sendbytes);
        if (sendbytes==0){
          send(link2, "OK", 2, 0);
        }
        else{
          send(link2, bufor2, sendbytes, 0);
        }
        if(sendbytes == 1024){
          send(link2, "OK", 2, 0);
        }
        memset(msgsx, 0, 5000);
        memcpy(shmem_msgsx, msgsx, sizeof(msgsx));
      }

      if(strcmp(bufor2, "DISCONNECT") == 0){
        send(link2, "OK", 2, 0);
        close(link2);
        close(socket2);
        break;
      }

      if(strcmp(bufor2, "SEND_FILE") == 0){
        bytes = 0;
        newbytes = 0;
        send(link2, "OK", 2, 0);
        memset(bufor2, 0, 1024);
        recv(link2, bufor2, 1023, 0);
        strcpy(fname, bufor2);
        printf("Nazwa Pliku: %s\n", bufor2);
        send(link2, "OK", 2, 0);
        memset(bufor2, 0, 1024);
        recv(link2, bufor2, 1023, 0);
        send(link2, "OK", 2, 0);
        filesize = atoi(bufor2);
        printf("Wielkość Pliku: %d\n", filesize);
        strcpy(fdest, dest);
        strcat(fdest, fname);
        f = fopen(fdest, "wb+");
        while(bytes < filesize){
          memset(bufor2, 0, 1024);
          newbytes = recv(link2, bufor2, 1023, 0);
          fwrite(bufor2, 1, strlen(bufor2), f);
          bytes += newbytes;
        }
        send(link2, "OK", 2, 0);
        fclose(f);
      }

      if(strcmp(bufor2, "GET_FILE") == 0){
        filesize = 0;
        send(link2, "OK", 2, 0);
        memset(bufor2, 0, 1024);
        recv(link2, bufor2, 1023, 0);
        strcpy(fname, bufor2);
        printf("Nazwa Pliku: %s\n", fname);
        memset(bufor2, 0, 1024);
        strcpy(fdest, dest);
        strcat(fdest, fname);
        memset(fname, 0, strlen(fname));
        if( access( fdest, F_OK ) != 0 ) {
          send(link2, "OK", 2, 0);
        }
        else{
          f = fopen(fdest, "r+");
          fseek(f, 0, SEEK_END);
          filesize = ftell(f);
          fseek(f, 0, SEEK_SET);
          data = malloc(1025 * sizeof(char));
          dataptr = 0;
          sendbytes = 0;
          sendbytes = filesize;
          while(sendbytes>1024){
              memset(data, 0, 1025);
              fread(data, 1024, 1, f);
              send(link2, data, 1024, 0);
              sendbytes = sendbytes - 1024;
          }
          if (sendbytes==0){
            send(link2, "OK", 2, 0);
          }
          else{
            memset(data, 0, 1025);
            fread(data, sendbytes, 1, f);
            send(link2, data, sendbytes, 0);
          }
          if(sendbytes == 1024){
            send(link2, "OK", 2, 0);
          }
          free(data);
          fclose(f);
        }

      }
    }
  }
  return 0;
}
