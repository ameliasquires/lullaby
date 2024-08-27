#include "common.h"
#include "util.h"

/**
 * @brief calls recv into buffer until everything is read
 *
*/
int64_t recv_full_buffer(int client_fd, char** _buffer, int* header_eof, int* state){
  char* header, *buffer = malloc(BUFFER_SIZE * sizeof * buffer);
  memset(buffer, 0, BUFFER_SIZE);
  int64_t len = 0;
  *header_eof = -1;
  int n, content_len = -1;
  //printf("&_\n");
  for(;;){
    n = recv(client_fd, buffer + len, BUFFER_SIZE, 0);
    if(n < 0){
      *_buffer = buffer;
      printf("%s %i\n",strerror(errno),errno);
      if(*header_eof == -1) return -2; //dont even try w/ request, no header to read
      return -1; //well the header is fine atleast

    };
    if(*header_eof == -1 && (header = strstr(buffer, "\r\n\r\n")) != NULL){
      *header_eof = header - buffer;
      char* cont_len_raw = strstr(buffer, "Content-Length: ");
      
      if(cont_len_raw == NULL) {
        len += n;
        *_buffer = buffer;
        return len;
      }
      
      str* cont_len_str = str_init("");
      if(cont_len_raw == NULL) abort();
      //i is length of 'Content-Length: '
      for(int i = 16; cont_len_raw[i] != '\r'; i++) str_pushl(cont_len_str, cont_len_raw + i, 1);
      content_len = strtol(cont_len_str->c, NULL, 10);
      str_free(cont_len_str);
      if(content_len > max_content_length) {
        *_buffer = buffer;
        *state = (len + n != content_len + *header_eof + 4);
        return len + n;
      }
      buffer = realloc(buffer, content_len + *header_eof + 4 + BUFFER_SIZE);
      if(buffer == NULL) p_fatal("unable to allocate");
    }

    len += n;
    if(*header_eof == -1){
      buffer = realloc(buffer, len + BUFFER_SIZE + 1);
      memset(buffer + len, 0, n + 1);
    }
    

    if(content_len != -1 && len - *header_eof - 4 >= content_len) break;
  }
  *_buffer = buffer;
  return len;
}

/**
 * @brief converts the request buffer into a parray_t
 *
*/
int parse_header(char* buffer, int header_eof, parray_t** _table){
  if(header_eof == -1) return -1;
  char add[] = {0,0};
  int lines = 3;
  for(int i = 0; i != header_eof; i++) lines += buffer[i] == '\n';
  parray_t* table = parray_init();
  str* current = str_init("");
  int oi = 0;
  int item = 0;
  for(; oi != header_eof; oi++){
    if(buffer[oi] == ' ' || buffer[oi] == '\n'){
      if(buffer[oi] == '\n') current->c[current->len - 1] = 0;
      parray_set(table, item == 0 ? "Request" :
                item == 1 ? "Path" : "Version", (void*)str_init(current->c));
      str_clear(current);
      item++;
      if(buffer[oi] == '\n') break;
    } else str_pushl(current, buffer + oi, 1);
  }

  int key = 1;
  str* sw = NULL;
  for(int i = oi + 1; i != header_eof; i++){
    if(buffer[i] == ' ' && strcmp(current->c, "") == 0) continue;
    if(key && buffer[i] == ':' || !key && buffer[i] == '\n'){
      if(key){
        sw = current;
        current = str_init("");
        key = 0;
      } else {
        if(buffer[oi] == '\n') current->c[current->len - 1] = 0;
        parray_set(table, sw->c, (void*)str_init(current->c));
        str_clear(current);
        str_free(sw);
        sw = NULL;
        key = 1;
      }
      continue;
    } else str_pushl(current, buffer + i, 1);
  }
  parray_set(table, sw->c, (void*)str_init(current->c));

  str_free(current);
  if(sw != NULL) str_free(sw);
  *_table = table;
  return 0;
}

/**
 * @brief contructs an http request
 *
*/
void http_build(str** _dest, int code, char* code_det, char* header_vs, char* content, size_t len){
  char* dest = malloc(HTTP_BUFFER_SIZE);
  memset(dest, 0, HTTP_BUFFER_SIZE);

  sprintf(dest, 
    "HTTP/1.1 %i %s\r\n"
    "%s"
    "\r\n"
    , code, code_det, header_vs);
  *_dest = str_init(dest);
  str_pushl(*_dest, content, len);

  free(dest);
}

/**
 * @brief gets a string representation of a http code
 * 
*/
void http_code(int code, char* code_det){
  //this was done with a script btw
  switch(code){
    case 100: sprintf(code_det,"Continue"); break;
    case 101: sprintf(code_det,"Switching Protocols"); break;
    case 102: sprintf(code_det,"Processing"); break;
    case 103: sprintf(code_det,"Early Hints"); break;
    case 200: sprintf(code_det,"OK"); break;
    case 201: sprintf(code_det,"Created"); break;
    case 202: sprintf(code_det,"Accepted"); break;
    case 203: sprintf(code_det,"Non-Authoritative Information"); break;
    case 204: sprintf(code_det,"No Content"); break;
    case 205: sprintf(code_det,"Reset Content"); break;
    case 206: sprintf(code_det,"Partial Content"); break;
    case 207: sprintf(code_det,"Multi-Status"); break;
    case 208: sprintf(code_det,"Already Reported"); break;
    case 226: sprintf(code_det,"IM Used"); break;
    case 300: sprintf(code_det,"Multiple Choices"); break;
    case 301: sprintf(code_det,"Moved Permanently"); break;
    case 302: sprintf(code_det,"Found"); break;
    case 303: sprintf(code_det,"See Other"); break;
    case 304: sprintf(code_det,"Not Modified"); break;
    case 307: sprintf(code_det,"Temporary Redirect"); break;
    case 308: sprintf(code_det,"Permanent Redirect"); break;
    case 400: sprintf(code_det,"Bad Request"); break;
    case 401: sprintf(code_det,"Unauthorized"); break;
    case 402: sprintf(code_det,"Payment Required"); break;
    case 403: sprintf(code_det,"Forbidden"); break;
    case 404: sprintf(code_det,"Not Found"); break;
    case 405: sprintf(code_det,"Method Not Allowed"); break;
    case 406: sprintf(code_det,"Not Acceptable"); break;
    case 407: sprintf(code_det,"Proxy Authentication Required"); break;
    case 408: sprintf(code_det,"Request Timeout"); break;
    case 409: sprintf(code_det,"Conflict"); break;
    case 410: sprintf(code_det,"Gone"); break;
    case 411: sprintf(code_det,"Length Required"); break;
    case 412: sprintf(code_det,"Precondition Failed"); break;
    case 413: sprintf(code_det,"Content Too Large"); break;
    case 414: sprintf(code_det,"URI Too Long"); break;
    case 415: sprintf(code_det,"Unsupported Media Type"); break;
    case 416: sprintf(code_det,"Range Not Satisfiable"); break;
    case 417: sprintf(code_det,"Expectation Failed"); break;
    case 418: sprintf(code_det,"I'm a teapot"); break;
    case 421: sprintf(code_det,"Misdirected Request"); break;
    case 422: sprintf(code_det,"Unprocessable Content"); break;
    case 423: sprintf(code_det,"Locked"); break;
    case 424: sprintf(code_det,"Failed Dependency"); break;
    case 425: sprintf(code_det,"Too Early"); break;
    case 426: sprintf(code_det,"Upgrade Required"); break;
    case 428: sprintf(code_det,"Precondition Required"); break;
    case 429: sprintf(code_det,"Too Many Requests"); break;
    case 431: sprintf(code_det,"Request Header Fields Too Large"); break;
    case 451: sprintf(code_det,"Unavailable For Legal Reasons"); break;
    case 500: sprintf(code_det,"Internal Server Error"); break;
    case 501: sprintf(code_det,"Not Implemented"); break;
    case 502: sprintf(code_det,"Bad Gateway"); break;
    case 503: sprintf(code_det,"Service Unavailable"); break;
    case 504: sprintf(code_det,"Gateway Timeout"); break;
    case 505: sprintf(code_det,"HTTP Version Not Supported"); break;
    case 506: sprintf(code_det,"Variant Also Negotiates"); break;
    case 507: sprintf(code_det,"Insufficient Storage"); break;
    case 508: sprintf(code_det,"Loop Detected"); break;
    case 510: sprintf(code_det,"Not Extended"); break;
    case 511: sprintf(code_det,"Network Authentication Required"); break;
    default: sprintf(code_det,"unknown");
  }
}

void client_fd_errors(int client_fd){
  if(client_fd>=0) return;

  switch(client_fd){
    case -1:
      p_fatal("client fd already closed\n");
    case -2:
      p_fatal("request was partial\n");
    default:
      p_fatal("unknown negative client_fd value");
  }
}

int content_disposition(str* src, parray_t** _dest){

  char* end = strnstr(src->c, ";", src->len);
  parray_t* dest = parray_init();
  if(end == NULL){
    parray_set(dest, "form-data", (void*)str_init(src->c));
    return 0;
  }
  str* temp = str_init("");
  str_pushl(temp, src->c, end - src->c);
  parray_set(dest, "form-data", (void*)temp);

  int len = end - src->c;
  char* buffer = end + 1;

  gen_parse(buffer, src->len - len, &dest);
  //printf("\n**\n");
  //for(int i = 0; i != dest->len; i++){
  //  printf("'%s : %s'\n",((str*)dest->P[i].key)->c,((str*)dest->P[i].value)->c);
  //}
  *_dest = dest;

  return 1;
}

int match_param(char* path, char* match){
  int pi, mi = pi = 0;
  int step = 0;
  //0 increment both
  //1 move match to find '/' or '\0'
  //2 move path to find '}'
  for(; path[pi] != '\0' && match[mi] != '\0';){
    if(step == 0){
      if(path[pi] == '{'){
        step = 1;
      } else {
        pi++;
        mi++;
      } 
    } else if (step == 1){
      if(match[mi] == '/'){
        printf("\n");
        step = 2;
      } else {
        printf("%c",match[mi]);
        mi++;
      }
    } else if (step == 2){
      if(path[pi] == '}'){
        step = 0;
      } else {
        pi++;
      }
    }
  }
  return 0;
}

parray_t* route_match(parray_t* paths, char* request){
  parray_t* out = parray_initl(paths->len);
  out->len = 0;

  for(int i = 0; i != paths->len; i++){
    match_param(paths->P[i].key->c, request);
    if(strcmp(request, paths->P[i].key->c) == 0){
      out->P[out->len] = paths->P[i];
      out->len++;

    }
  }
  return out;
}
