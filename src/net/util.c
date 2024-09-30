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
    if(len >= MAX_HEADER_SIZE){
      *_buffer = buffer;
      return -2;//p_fatal("too large");
    } 
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
    } else {
      str_pushl(current, buffer + oi, 1);
    }
  }

  if(item != 3){
    *_table = table;
    return -1;
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
  if(sw != NULL){
    parray_set(table, sw->c, (void*)str_init(current->c));
    str_free(sw);
  }

  str_free(current);
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
          name = calloc(pi - start, sizeof * name);
          memcpy(name, path + start + 1, pi - start - 1);
          start = mi;
        }
        pi++;
        break;
      case GET_VALUE:
        //change this to maybe match the next path char?
        if(match[mi] == '/'){
          step = NORMAL;

          char* out = calloc(mi - start, sizeof * out);
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
    char* out = calloc(mi - start, sizeof * out);
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
void parse_mimetypes(){
  mime_type = map_init();
  FILE* fp = fopen(MIMETYPES, "r");
  char* buffer = calloc(1024, sizeof * buffer);

  for(;fgets(buffer, 1024, fp); memset(buffer, 0, 1024)){
    int i;
    for(i = 0; buffer[i] == ' '; i++);
    if(buffer[i] == '#') continue;

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
