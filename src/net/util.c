#include "common.h"
#include "util.h"
#include "ssl.h"
#include <ctype.h>

int64_t recv_header(struct net_data* data, char** _buffer, char** header_eof){
  char* buffer = calloc(sizeof* buffer, BUFFER_SIZE);
  *_buffer = buffer;
  int64_t len = 0;
  int64_t n = 0;
  *header_eof = 0;

  for(;;){
    n = net_ctx_read(data, buffer + len, BUFFER_SIZE);

    if(n <= 0){
      //printf("%s %i\n", strerror(errno), errno);
      return -1;
    }

    // search the last 4 characters too if they exist
    // this could probably be changed to 3
    int64_t start_len = len - 4 > 0 ? len - 4 : len;
    int64_t search_end = len - 4 > 0 ? n + 4 : n;
    if((*header_eof = memmem(buffer + start_len, search_end, "\r\n\r\n", 4)) != NULL){
      return len + n; 
    }

    if((len += n) >= MAX_HEADER_SIZE){
      return -2;
    }

    buffer = realloc(buffer, sizeof* buffer * (len + BUFFER_SIZE + 1));

    *_buffer = buffer;
  }
}

void lowercase(char* c, uint64_t len){
  for(int i = 0; i != len; i++)
    c[i] = tolower(c[i]);
}

#define max_uri_len 4096
/**
 * @brief converts the request buffer into a parray_t
 *
 */
ssize_t parse_header_head(char* buffer, size_t header_len, parray_t* table){
  str* current = str_init("");
  int i = 0;
  int item = 0;

  for(; i != header_len; i++){
    if(buffer[i] == ' ' || buffer[i] == '\n'){
      if(buffer[i] == '\n') current->c[current->len - 1] = 0;
      if(item < 3) parray_set(table, item == 0 ? "request" :
          item == 1 ? "path" : "version", (void*)str_init(current->c));
      str_clear(current);
      item++;
      if(buffer[i] == '\n') break;
    } else {
      if(i >= max_uri_len){
        str_free(current);
        return -2;
      }
      str_pushl(current, buffer + i, 1);
    }
  }

  str_free(current);

  if(item < 3){
    return -1;
  }

  return i;
}

int parse_header_kv(char* buffer, size_t header_len, parray_t* table){
  str* current = str_init("");
  str* sw = NULL;
  int key = 1;

  for(int i = 0; i != header_len; i++){
    if(buffer[i] == ' ' && strcmp(current->c, "") == 0) continue;
    if((key && buffer[i] == ':') || (!key && buffer[i] == '\n')){
      if(key){
        sw = current;
        current = str_init("");
        key = 0;
      } else {
        current->c[current->len - 1] = 0;
        //duplicate keys would cause memory leaks, ignore them for now
        //todo: figure out system to handle this
        str* id = (str*)parray_get(table, sw->c);
        if(id != NULL) str_free(id);
        lowercase(sw->c, sw->len);
        parray_set(table, sw->c, (void*)str_init(current->c));
        str_clear(current);
        str_free(sw);
        sw = NULL;
        key = 1;
      }
      continue;
    } else str_pushc(current, buffer[i]);
  }

  if(sw != NULL){
    lowercase(sw->c, sw->len);
    parray_set(table, sw->c, (void*)str_init(current->c));
    str_free(sw);
  }
  str_free(current);

  return 0;
}

int parse_header(char* buffer, size_t header_len, parray_t** _table){
  if(header_len == -1) return -1;
  parray_t* table = parray_init();

  ssize_t used = parse_header_head(buffer, header_len, table);
  if(used == -1) return -1;

  parse_header_kv(buffer + used + 1, header_len - used, table);
  *_table = table;
  return 0;
}


/**
 * @brief contructs an http request
 *
 */
void http_build(str** _dest, int code, const char* code_det, char* header_vs, char* content, size_t len){
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
const char* http_code(int code){
  switch(code){
    case 100: return "Continue"; break;
    case 101: return "Switching Protocols"; break;
    case 102: return "Processing"; break;
    case 103: return "Early Hints"; break;
    case 200: return "OK"; break;
    case 201: return "Created"; break;
    case 202: return "Accepted"; break;
    case 203: return "Non-Authoritative Information"; break;
    case 204: return "No Content"; break;
    case 205: return "Reset Content"; break;
    case 206: return "Partial Content"; break;
    case 207: return "Multi-Status"; break;
    case 208: return "Already Reported"; break;
    case 226: return "IM Used"; break;
    case 300: return "Multiple Choices"; break;
    case 301: return "Moved Permanently"; break;
    case 302: return "Found"; break;
    case 303: return "See Other"; break;
    case 304: return "Not Modified"; break;
    case 307: return "Temporary Redirect"; break;
    case 308: return "Permanent Redirect"; break;
    case 400: return "Bad Request"; break;
    case 401: return "Unauthorized"; break;
    case 402: return "Payment Required"; break;
    case 403: return "Forbidden"; break;
    case 404: return "Not Found"; break;
    case 405: return "Method Not Allowed"; break;
    case 406: return "Not Acceptable"; break;
    case 407: return "Proxy Authentication Required"; break;
    case 408: return "Request Timeout"; break;
    case 409: return "Conflict"; break;
    case 410: return "Gone"; break;
    case 411: return "Length Required"; break;
    case 412: return "Precondition Failed"; break;
    case 413: return "Content Too Large"; break;
    case 414: return "URI Too Long"; break;
    case 415: return "Unsupported Media Type"; break;
    case 416: return "Range Not Satisfiable"; break;
    case 417: return "Expectation Failed"; break;
    case 418: return "I'm a teapot"; break;
    case 421: return "Misdirected Request"; break;
    case 422: return "Unprocessable Content"; break;
    case 423: return "Locked"; break;
    case 424: return "Failed Dependency"; break;
    case 425: return "Too Early"; break;
    case 426: return "Upgrade Required"; break;
    case 428: return "Precondition Required"; break;
    case 429: return "Too Many Requests"; break;
    case 431: return "Request Header Fields Too Large"; break;
    case 451: return "Unavailable For Legal Reasons"; break;
    case 500: return "Internal Server Error"; break;
    case 501: return "Not Implemented"; break;
    case 502: return "Bad Gateway"; break;
    case 503: return "Service Unavailable"; break;
    case 504: return "Gateway Timeout"; break;
    case 505: return "HTTP Version Not Supported"; break;
    case 506: return "Variant Also Negotiates"; break;
    case 507: return "Insufficient Storage"; break;
    case 508: return "Loop Detected"; break;
    case 510: return "Not Extended"; break;
    case 511: return "Network Authentication Required"; break;
    default: return "unknown";
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

#warning "leak, last calloc"
int match_param(char* path, char* match, parray_t* arr){
  int pi, index, imatch, start, mi;
  mi = pi = imatch = start = 0;
  index = -1;

  enum steps {
    NORMAL,
    GET_KEY,
    GET_VALUE
  };

  enum steps step = NORMAL;
  char* name;

  for(; /*(path[pi] != '\0' || step == 2) && */(match[mi] != '\0' || step == 1);){
    switch(step){
      case NORMAL:
        if(path[pi] == '{'){

          step = GET_KEY;
          start = pi;
        } else if(path[pi] == '*'){
          index = pi;
          imatch = mi;

        } else {

          if(path[pi] != match[mi]){
            if(index == -1) return 0;

            pi = index + 1;
            imatch++;
            mi = imatch;
            continue;
          }
          mi++;
        } 
        pi++;
        break;
      case GET_KEY:
        if(path[pi] == '}'){
          step = GET_VALUE;
          name = calloc(pi - start + 1, sizeof * name);
          memcpy(name, path + start + 1, pi - start - 1);
          start = mi;
        }
        pi++;
        break;
      case GET_VALUE:
        //change this to maybe match the next path char?
        if(match[mi] == '/'){
          step = NORMAL;

          char* out = calloc(mi - start + 1, sizeof * out);
          memcpy(out, match + start, mi - start);
          parray_set(arr, name, out);
          free(name);
        } else {
          mi++;
        }
        break;
    }
  }

  if(step == GET_VALUE){
    char* out = calloc(mi - start + 1, sizeof * out);
    memcpy(out, match + start, mi - start);
    parray_set(arr, name, out);
    free(name);
  }

  if(path[pi] != 0) for(; path[pi] == '*'; pi++);

  return path[pi] == 0 && match[mi] == 0;
}

parray_t* route_match(parray_t* paths, char* request, larray_t** _params){
  larray_t* params = *_params;
  parray_t* out = parray_initl(paths->len * 2);
  parray_t* temp;
  out->len = 0;

  for(int i = 0; i != paths->len; i++){
    //if(match_param(paths->P[i].key->c, request))
    //*if(strcmp(request, paths->P[i].key->c) == 0)*/{
    //printf("pass!\n");
    //printf("%i\n", i);

    temp = parray_init();

    if(match_param(paths->P[i].key->c, request, temp)){
      out->P[out->len] = paths->P[i];
      larray_set(&params, out->len, (void*)temp);
      out->len++;
    } else {
      parray_clear(temp, FREE);
    }

    //}
  }

  *_params = params;
  return out;
}

map_t* mime_type = NULL;
char* _mimetypes = "/etc/mime.types";
size_t _mimetypes_len = 15;

void parse_mimetypes(){
  if(_mimetypes == NULL || _mimetypes_len == 0) return;
  if(mime_type != NULL) return;
  mime_type = map_init();

  FILE* fp = fopen(_mimetypes, "r");
  if(fp == NULL){
    fprintf(stderr, "unable to load mimetypes, set llby.net.mimetypes to a proper location, or nil to skip this\n");
    return;
  }

  char* line = NULL;
  size_t len = 0;
  ssize_t read;

  for(;(read = getline(&line, &len, fp)) != -1;){
    if(line[0] == '#' || line[0] == '\n') continue;

    char* mtype = calloc(1024, sizeof * mtype);
    int mtype_len = 0;
    int i = 0;
    for(; line[i] != ' ' && line[i] != '\t' && i < read; i++){
      mtype[mtype_len] = line[i];
      mtype_len++;
    }

    char* type = calloc(512, sizeof * type);
    int type_len = 0;
    for(; i < read; i++){
      if(line[i] == ' ' || line[i] == '\t' || line[i] == '\n'){
        if(type_len == 0) continue;
        char* mtype_c = calloc(1024, sizeof * mtype);
        mtype_c = strcpy(mtype_c, mtype);
        map_set(&mime_type, type, mtype_c);
        type_len = 0;
        free(type);
        type = calloc(512, sizeof * type);
      } else {
        type[type_len] = line[i];
        type_len++;
      }
    }
    free(mtype);
    free(type);
  }

  free(line);
  fclose(fp);
}

void _parse_mimetypes(){
  abort();
  mime_type = map_init();
  FILE* fp = fopen(MIMETYPES, "r");
  char* buffer = calloc(1024, sizeof * buffer);

  for(;fgets(buffer, 1024, fp); memset(buffer, 0, 1024)){
    int i;
    for(i = 0; buffer[i] == ' '; i++);
    if(buffer[i] == '#') {
      for(; buffer[i] != '\n' && buffer[i] != '\0'; i++);
      continue;
    }

    //printf("s: '%s'\n",buffer + i);
    char* type = calloc(512, sizeof * type);
    int type_len = 0;
    for(; buffer[i + type_len] != ' ' && buffer[i + type_len] != '\t'; type_len++){
      //printf("%c", buffer[i + type_len]);
      type[type_len] = buffer[i + type_len];
      if(buffer[i + type_len] == '\0' || buffer[i + type_len] == '\n') break;
    }
    type[type_len] = '\0';

    //check if the type has an associated file type
    if(buffer[i + type_len] == '\0' || buffer[i + type_len] == '\n'){
      free(type);
      continue;
    }
    type = realloc(type, (type_len + 1) * sizeof * type);


    int file_type_len = 0;
    char* file_type = calloc(512, sizeof * file_type);
    i += type_len;

    for(;buffer[i] == ' ' || buffer[i] == '\t'; i++);

    int used = 0;
    for(;;){
      if(buffer[i + file_type_len] == ' ' || buffer[i + file_type_len] == '\n' || buffer[i + file_type_len] == '\0'){
        used = 1;
        file_type[file_type_len] = '\0';
        file_type = realloc(file_type, (file_type_len + 1) * sizeof * file_type);
        //printf("set %s to %s\n",file_type, type);
        map_set(&mime_type, file_type, type);
        file_type = calloc(512, sizeof * file_type);
        if(buffer[i + file_type_len] != ' ') break;
        i += file_type_len + 1;
        file_type_len = 0;
        //printf("finish\n");
      } else {
        file_type[file_type_len] = buffer[i + file_type_len];
        file_type_len++;
      }
    }
    free(file_type);

    //printf("e: '%s'\n", type);
    if(!used)free(type);
  }
  //printf("done\n");

}

int net_error(struct net_data* ctx, int code){
  char out[512] = {0};
  str* s = str_init(http_code(code));
  str_lowercase(s);

  sprintf(out, "HTTP/1.1 %i %s\n\n%s", code, http_code(code), s->c);
  free(s);
  net_ctx_write(ctx, out, strlen(out));
  return 0;
}

int percent_decode(str* input, str** _output){
  str* output = str_init("");

  //could maybe make this better by using memmem to find occurrences of %
  for(int i = 0; i <= input->len; i++){
    if(input->c[i] == '%' && input->len - i >= 3){
      str* hex = str_initfl(input->c + i + 1, 2); 

      //casting a long to a char pointer is a horrible idea
      long c = strtol(hex->c, NULL, 16);
      if(c == 0){
        str_free(hex);
        *_output = output;
        return 1;
      }

      if(c == '/') c = '%';
      str_pushl(output, ((char*)&c), 1);
      str_free(hex);
      i += 2;
    } else {
      str_pushl(output, input->c + i, 1);
    }
  }
  *_output = output; 
  return 0;
}

int net_ctx_read(struct net_data* data, void* buffer, size_t c){
  if(data->ssl == NULL){
    return read(data->sock, buffer, c);
  }
  return SSL_read(data->ssl, buffer, c);
}

int net_ctx_write(struct net_data* data, void* buffer, size_t c){
  if(data->ssl == NULL){
    return write(data->sock, buffer, c);
  }
  return SSL_write(data->ssl, buffer, c);
}

int net_ctx_close(struct net_data* data){
  if(data->sock != -1){
    if(data->ssl != NULL) SSL_shutdown(data->ssl);
    else {
      shutdown(data->sock, 2);
      closesocket(data->sock);
    }
  }

  data->sock = -1;
  free(data);

  return 0;
}
