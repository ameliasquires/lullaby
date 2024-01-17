#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>

#include "src/i_str.h"

#define max_con 10
#define BUFFER_SIZE 2048

size_t recv_full_buffer(int client_fd, char** _buffer, int* header_eof){
  char* buffer = malloc(BUFFER_SIZE * sizeof * buffer);
  char* header;
  size_t len = 0;
  *header_eof = -1;
  int n;

  for(;;){
    n = recv(client_fd, buffer + len, BUFFER_SIZE, 0);
    if(*header_eof == -1 && (header = strstr(buffer, "\r\n\r\n")) != NULL){
      *header_eof = header - buffer;
    }
    if(n != BUFFER_SIZE) break;
    len += BUFFER_SIZE;
    buffer = realloc(buffer, len + BUFFER_SIZE);
  }
  buffer[n] = 0;

  *_buffer = buffer;
  return len + BUFFER_SIZE;
}

int parse_header(char* buffer, int header_eof, str*** _table, int* _len){
  if(header_eof == -1) return -1;
  char add[] = {0,0};
  int lines = 3;
  for(int i = 0; i != header_eof; i++) lines += buffer[i] == '\n';
  str** table = malloc(sizeof ** table * lines * 2);
  table[0] = str_init("Request");// table[1] = str_init("Post|Get");
  table[2] = str_init("Path");// table[3] = str_init("/");
  table[4] = str_init("Version");// table[5] = str_init("HTTP/1.1");
  str* current = str_init("");
  int ins = 1;
  for(int i = 0; i != header_eof; i++){
    add[0] = buffer[i];
    if(buffer[i] == '\n') break;
    if(buffer[i] == ' '){
      table[ins] = str_init(current->c);
      ins += 2;
      str_clear(current);
    } else str_push(current, add);
  }
  table[ins] = str_init(current->c);

  int tlen = 6;

  int key = 1;
  for(int i = (strstr(buffer,"\n") - buffer) + 1; i != header_eof; i++){
    if(key && buffer[i]==':' || !key && buffer[i]=='\n') {
      table[tlen] = str_init(current->c);
      str_clear(current);
      tlen++;
      i+=key;
      key = !key;
      continue;
    }
    add[0] = buffer[i];
    str_push(current, add);
  }
  table[tlen] = str_init(current->c);
  tlen++;
  str_free(current);
  *_len = tlen / 2;
  *_table = table; 
  return 0;
}

void* handle_client(void *arg){
  int client_fd = *((int*)arg);
  char* buffer;
  int header_eof;
  size_t bytes_received = recv_full_buffer(client_fd, &buffer, &header_eof);
  //printf("%lu %i %s\n",bytes_received, header_eof, buffer); 
  if(bytes_received > 0){
    str** table;
    int len = 0;
    if(parse_header(buffer, header_eof, &table, &len) != -1){

      printf("%i\n",len); 
      for(int i = 0; i != len * 2; i+=2){
        printf("%s :: %s\n",table[i]->c, table[i+1]->c);
      }
    }
  }
  free(buffer);
  return NULL;
}

int main(){
  int server_fd;
  struct sockaddr_in server_addr;

  if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    printf("error\n");
    abort();
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(3042);

  if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
    printf("failed to bind\n");
    abort();
  }

  if(listen(server_fd, max_con) < 0){
    printf("failed to listen\n");
    abort();
  }

  for(;;){
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int* client_fd = malloc(sizeof(int));

    if((*client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len)) < 0){
      printf("failed to accept\n");
      abort();
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, (void*)client_fd);
    pthread_detach(thread_id);
  }

}
