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



static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};


void build_decoding_table() {

    decoding_table = malloc(256);

    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length) {

    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = malloc(*output_length);
    if (encoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}


unsigned char *base64_decode(const char *data,
                             size_t input_length,
                             size_t *output_length) {

    if (decoding_table == NULL){ build_decoding_table();}

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (int i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}




void base64_cleanup() {
    free(decoding_table);
}


int gameCheck (char tab[]){
  if((tab[0]==tab[1] && tab[1]==tab[2] && tab[0]!='0') || (tab[0]==tab[4] && tab[4]==tab[8] && tab[0]!='0') || (tab[0]==tab[3] && tab[3]==tab[6] && tab[0]!='0') || (tab[3]==tab[4] && tab[4]=
=tab[5] && tab[3]!='0') || (tab[1]==tab[4] && tab[4]==tab[7] && tab[1]!='0') || (tab[2]==tab[5] && tab[5]==tab[8] && tab[2]!='0') || (tab[6]==tab[7] && tab[7]==tab[8] && tab[6]!='0') || (tab
[2]==tab[4] && tab[4]==tab[6] && tab[2]!='0')){
    return 1;
  }
  else{
    return 0;
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
  char bufor1[1024], bufor2[1024];
  char dest[1024];
  strcpy(dest, "/home/students/s452664/TicTacToe-Project/files/");
  char fdest[1024], fname[1024];
  int socket1, socket2, link1, link2;
  int msgs_size, msgptr;
  int dataptr;
  int sendbytes;
  size_t *encodesize;
  int bytes, newbytes, filesize;
  struct sockaddr_in adr1, adr2, nadawca1, nadawca2;
  FILE *f;

  struct timeval tm;
  int active_player;
  gettimeofday(&tm, NULL);
  srandom(tm.tv_sec + tm.tv_usec * 1000000ul);

  int gameboard[9];
  char gamesend[19];
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

  char *msgscated;
  char *file;
  char *data;
  char msgsx [5000];
  memset(msgsx, 0, 5000);
  char msgso [5000];
  memset(msgso, 0, 5000);

  void* shmem_msgx = create_shared_memory(20);
  void* shmem_msgo = create_shared_memory(20);
  void* shmem_gamesend = create_shared_memory(20);
  void* shmem_gameboard = create_shared_memory(10);
  void* shmem_msgsx = create_shared_memory(5000);
  void* shmem_msgso = create_shared_memory(5000);

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

  memset(bufor1, 0, 1024);
  memset(bufor2, 0, 1024);
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
      }

      if(strcmp(bufor1, "GETMSG_O") == 0){
        msgs_size = 0;
        msgptr = 0;
        sendbytes = 0;
        memcpy(msgso, shmem_msgso, sizeof(msgso));
        memset(bufor1, 0, 1024);
        sendbytes = strlen(msgso);
        msgptr = 0;
        printf("%d %s\n", sendbytes, msgso);
        while(sendbytes>48){
          memcpy(bufor1, &msgso[0]+msgptr, 48);
          printf("%s\n", bufor1);
          send(link1, bufor1, 48, 0);
          msgptr = msgptr + 48;
          sendbytes = sendbytes - 48;
        }
        memcpy(bufor1, &msgso[0]+msgptr, sendbytes);
        if (sendbytes==0){
          send(link1, "OK", 2, 0);
        }
        else{
          send(link1, bufor1, sendbytes, 0);
        }
        if(sendbytes == 48){
          send(link1, '\0', 1, 0);
        }
        memset(msgso, 0, 5000);
        memcpy(shmem_msgso, msgso, sizeof(msgso));
      }
      if(strcmp(bufor1, "DISCONNECT") == 0){
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
        data = malloc(filesize * sizeof(char));
        strcpy(fdest, dest);
        strcat(fdest, fname);
        f = fopen(fdest, "wb+");
        while(bytes < filesize){
          memset(bufor1, 0, 1024);
          newbytes = recv(link1, bufor1, 1023, 0);
          printf("File: %s\n", bufor1);
          strcat(data, bufor1);
          bytes += newbytes;
        }
        printf("%s\n", data);
        fwrite(data, 1, sizeof(data), f);
        send(link1, "OK", 2, 0);
        fclose(f);
        free(data);
      }
      if(strcmp(bufor1, "GET_FILE") == 0){
        filesize = 0;
        send(link1, "OK", 2, 0);
        memset(bufor1, 0, 1024);
        recv(link1, bufor1, 1023, 0);
        strcpy(fname, bufor2);
        printf("Nazwa Pliku: %s\n", fname);
        memset(bufor1, 0, 1024);
        strcpy(fdest, dest);
        strcat(fdest, fname);
        memset(fname, 0, strlen(fname));
        f = fopen(fdest, "wb+");
        fseek(f, 0, SEEK_END);
        filesize = ftell(f);
        fseek(f, 0, SEEK_SET);
        data = malloc(filesize+1 * sizeof(char));
        memset(data, 0, filesize+1);
        fread(data, filesize+1, 1, f);

        dataptr = 0;
        sendbytes = 0;
        sendbytes = strlen(data);
        while(sendbytes>48){
          memcpy(bufor1, &data[0]+dataptr, 48);
          send(link1, bufor1, 48, 0);
          dataptr = dataptr + 48;
          sendbytes = sendbytes - 48;
        }
        memcpy(bufor1, &data[0]+msgptr, sendbytes);
        if (sendbytes==0){
          send(link1, "OK", 2, 0);
        }
        else{
          send(link1, bufor1, sendbytes, 0);
        }
        if(sendbytes == 48){
          send(link1, "OK", 2, 0);
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
        memset(bufor2, 0, 1024);
        sendbytes = strlen(msgsx);
        printf("%d %s\n", sendbytes, msgsx);
        while(sendbytes>48){
          memcpy(bufor2, &msgsx[0]+msgptr, 48);
          printf("%s\n", bufor2);
          send(link2, bufor2, 48, 0);
          msgptr = msgptr + 48;
          sendbytes = sendbytes - 48;
        }
        memcpy(bufor2, &msgsx[0]+msgptr, sendbytes);
        if (sendbytes==0){
          send(link2, "OK", 2, 0);
        }
        else{
          send(link2, bufor2, sendbytes, 0);
        }
        if(sendbytes == 48){
          send(link2, "OK", 2, 0);
        }
        memset(msgsx, 0, 5000);
        memcpy(shmem_msgsx, msgsx, sizeof(msgsx));
      }

      if(strcmp(bufor2, "DISCONNECT") == 0){
        close(link2);
        close(socket2);
        break;
      }
      if(strcmp(bufor2, "SEND_FILE") == 0){
        bytes = 0;
        newbytes = 0;
        filesize = 0;
        send(link2, "OK", 2, 0);
        memset(bufor2, 0, 1024);
        recv(link2, bufor2, 1023, 0);
        strcpy(fname, bufor2);
        printf("Nazwa Pliku: %s\n", fname);
        send(link2, "OK", 2, 0);
        memset(bufor2, 0, 1024);
        recv(link2, bufor2, 1023, 0);
        send(link2, "OK", 2, 0);
        filesize = atoi(bufor2);
        printf("Wielkość Pliku: %d\n", filesize);
        data = malloc(filesize * sizeof(char));
        memset(data, 0, 1024);
        strcpy(fdest, dest);
        strcat(fdest, fname);
        memset(fname, 0, strlen(fname));
        f = fopen(fdest, "wb+");
        while(bytes < filesize){
          memset(bufor2, 0, 1024);
          newbytes = recv(link2, bufor2, 1023, 0);
          printf("File: %s\n", bufor2);
          strcat(data, bufor2);
          bytes += newbytes;
        }
        memset(fdest, 0, strlen(fdest));
        fwrite(data, 1, sizeof(data), f);
        send(link2, "OK", 2, 0);
        fclose(f);
        free(data);
        free(file);
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
        f = fopen(fdest, "wb+");
        fseek(f, 0, SEEK_END);
        filesize = ftell(f);
        fseek(f, 0, SEEK_SET);
        data = malloc(filesize+1 * sizeof(char));
        memset(data, 0, filesize+1);
        fread(data, filesize+1, 1, f);

        dataptr = 0;
        sendbytes = 0;
        sendbytes = strlen(data);
        while(sendbytes>48){
          memcpy(bufor2, &data[0]+dataptr, 48);
          send(link2, bufor2, 48, 0);
          dataptr = dataptr + 48;
          sendbytes = sendbytes - 48;
        }
        memcpy(bufor2, &data[0]+msgptr, sendbytes);
        if (sendbytes==0){
          send(link2, "OK", 2, 0);
        }
        else{
          send(link2, bufor2, sendbytes, 0);
        }
        if(sendbytes == 48){
          send(link2, "OK", 2, 0);
        }

      }
    }
  }

  close(link1);
  close(link2);
  close(socket1);
  close(socket2);
  return 0;
}
