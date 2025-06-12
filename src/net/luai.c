#include "luai.h"
#include "common.h"
#include "util.h"

void i_write_header(lua_State* L, int header_top, str** _resp, char* content, size_t len){
  str* resp;
  lua_pushvalue(L, header_top);

  str* header_vs = str_init("");
  lua_pushnil(L);
    
  for(;lua_next(L, header_top) != 0;){
      char* key = (char*)luaL_checklstring(L, -2, NULL);
      if(strcmp(key, "Code") != 0){
        str_push(header_vs, key);
        str_push(header_vs, ": ");
        str_push(header_vs, (char*)luaL_checklstring(L, -1, NULL));
        str_push(header_vs, "\r\n");
      }
      lua_pop(L, 1);
  }

  lua_pushvalue(L, header_top);
  lua_pushstring(L, "Code");
  lua_gettable(L, header_top);
  int code = luaL_checkinteger(L, -1);
    
  const char* code_det = http_code(code);
  http_build(&resp, code, code_det, header_vs->c, content, len);

  str_free(header_vs);
  *_resp = resp;
}

/**
 * @brief parses all files in response buffer into a lua table
 *
 * @param {lua_State*} lua state to put table into
 * @param {char*} response buffer
 * @param {str*} response header Content-Type value
 * @return {int} lua index of table
*/
int rolling_file_parse(lua_State* L, int* files_idx, int* body_idx, char* buffer, str* content_type, size_t blen, struct file_parse* _content){
  struct file_parse content = *_content;
  /*enum file_status* status = (enum file_status*)parray_get(content, "_status");
  str* current = (str*)parray_get(content, "_current");
  str* old = (str*)parray_get(content, "_old");
  str* boundary = (str*)parray_get(content, "_boundary");
  str* boundary_id = (str*)parray_get(content, "_boundary_id");
  int* dash_count = (int*)parray_get(content, "_dash_count");
  int* table_idx = (int*)parray_get(content, "_table_idx");*/
  int override = 0;

  //time_start(start)
  if(content.status == _ignore){
    content.boundary = str_init(""); //usually add + 2 to the length when using
    int state = 0;
    for(int i = 0; content_type->c[i] != '\0'; i++){
      if(state == 2 && content_type->c[i] != '-') str_pushl(content.boundary, content_type->c + i, 1);
      if(content_type->c[i] == ';') state = 1;
      if(content_type->c[i] == '=' && state == 1) state = 2;
    }
    if(state == 2){
      str_pushl(content.boundary, "\r\n\r\n", 4);
    }

    content.status = state==2?BARRIER_READ:NORMAL;//malloc(sizeof * status); content.status = state==2?BARRIER_READ:NORMAL;
    content.dash_count = 0;//malloc(sizeof * dash_count); *dash_count = 0;
    content.current = str_init("");

    content.table_idx = lua_gettop(L);//malloc(sizeof * table_idx); *table_idx = lua_gettop(L);
    //parray_set(content, "_table_idx", (void*)(table_idx));
    //parray_set(content, "_status", (void*)(status));
    //parray_set(content, "_dash_count", (void*)(dash_count));
    //parray_set(content, "_current", (void*)(current));
    
    content.boundary_id = str_init("");

    //quick fix?
    //str_popb(content.boundary, 4);
    if(content.boundary->len >= 4) str_popb(content.boundary, 4);

    //parray_set(content, "_boundary", (void*)boundary);
    //parray_set(content, "_boundary_id", (void*)boundary_id);
    
  }
  //time_end("start", start)
  //printf("hi\n");
  if(content.status == NORMAL){
    //strnstr(buffer, )
    //if(override) str_clear(current);
    //str_pushl(current, buffer, blen);
    //printf("%s\n",current->c);
    lua_pushvalue(L, *body_idx);
    lua_pushlstring(L, buffer, blen);
    lua_concat(L, 2);
    *body_idx = lua_gettop(L);
  } else {
  file_start:;
    //time_start(barrier_read)
    if(content.status == BARRIER_READ){
      //printf("read %llu\n", blen);
      for(int i = 0; i != blen; i++){
        //printf("%c",buffer[i]);
        //printf("\n");
        if(*buffer == '\r'){
          content.status = FILE_HEADER;
          buffer += 2;
          blen -= i + 2;
          
          content.table_idx = lua_rawlen(L, *files_idx) + 1;
          lua_pushinteger(L, content.table_idx);
          lua_newtable(L);
          lua_settable(L, *files_idx);
          break;
        }
        str_pushl(content.boundary_id, buffer, 1);
        buffer++;
      }
    }
    //time_end("barrier_read", barrier_read)
    lua_pushvalue(L, *files_idx);
    lua_pushinteger(L, content.table_idx);
    lua_gettable(L, -2);
    int rfiles_idx = lua_gettop(L);
    //time_start(file_header)
    if(content.status == FILE_HEADER){
      //printf("header\n");
      for(int i = 0; i < blen; i++){

        if(buffer[i] == ':'){
          content.old = content.current;
          content.current = str_init("");
        } else if(buffer[i] == '\n'){
          if(content.current->len == 0){
            content.status = FILE_BODY;

            buffer += i + 1;
            blen -= i + 1;

            content.old = NULL;
            str_free(content.current);
            content.current = str_init("");
            break;
          }
          //printf("%i '%s' : '%s'\n",*table_idx, old->c, current->c);

          luaI_tsets(L, rfiles_idx, content.old->c, content.current->c);
          
          str_free(content.old);
          content.old = NULL;
          str_clear(content.current);
        } else if(buffer[i] != '\r' && !(buffer[i] == ' ' && content.current->len == 0)) str_pushl(content.current, buffer + i, 1);
      }
    } 
    //time_end("file_header", file_header)
    //time_start(file_body)
    if(content.status == FILE_BODY){
      char* barrier_end = memmem(buffer, blen, content.boundary->c, content.boundary->len);
      if(barrier_end == NULL){
        str* temp = str_initl(content.current->c, content.current->len);
        str_pushl(temp, buffer, blen); 
        barrier_end = memmem(temp->c, temp->len, content.boundary->c, content.boundary->len);
        if(barrier_end != NULL) abort(); // todo

        str* temp2 = content.current;
        content.current = temp;
        str_free(temp2);

      } else {
        char* start = barrier_end, *end = barrier_end;
        for(; *start != '\n'; start--);
        for(; *end != '\n'; end++);
        int clen = start - buffer;
        str_pushl(content.current, buffer, clen);
        luaI_tsetsl(L, rfiles_idx, "content", content.current->c, content.current->len - 1);
        str_clear(content.current);
        blen-= end - buffer;
        buffer = end;
        content.status = BARRIER_READ;
        goto file_start;
        //printf("%s\n",content.current->c);
      }
      
    }
    //time_end("file_body", file_body)
  }
  /*parray_set(content, "_dash_count", dash_count);
  parray_set(content, "_boundary_id", boundary_id);
  parray_set(content, "_boundary", boundary);
  parray_set(content, "_status", status);
  parray_set(content, "_current", current);
  parray_set(content, "_old", old);*/

  *_content = content;

  return 0;
}
