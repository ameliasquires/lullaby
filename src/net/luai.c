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
    if(strcmp(key, "code") != 0){
      str_push(header_vs, key);
      str_push(header_vs, ": ");
      str_push(header_vs, (char*)luaL_checklstring(L, -1, NULL));
      str_push(header_vs, "\r\n");
    }
    lua_pop(L, 1);
  }

  lua_pushvalue(L, header_top);
  lua_pushstring(L, "code");
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
int http_body_parse(lua_State* L, int* files_idx, int* body_idx, char* buffer, str* content_type, size_t blen, struct file_parse* _content){
  struct file_parse content = *_content;

  if(content.status == _ignore){

    parray_t* par = parray_init();
    gen_parse(content_type->c, content_type->len, &par);
    content.boundary = parray_pop(par, "boundary");
    content.status = content.boundary == NULL?BARRIER_READ:NORMAL;
    content.dash_count = 0;
    content.current = str_init("");
    content.table_idx = lua_gettop(L);
    content.boundary_id = str_init("");
    parray_clear(par, STR);

  }
  if(content.status == NORMAL){
    lua_pushvalue(L, *body_idx);
    lua_pushlstring(L, buffer, blen);
    lua_concat(L, 2);
    *body_idx = lua_gettop(L);
  } else {
file_start:;
           if(content.status == BARRIER_READ){
             for(int i = 0; i != blen; i++){
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
           lua_pushvalue(L, *files_idx);
           lua_pushinteger(L, content.table_idx);
           lua_gettable(L, -2);
           int rfiles_idx = lua_gettop(L);
           if(content.status == FILE_HEADER){
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
#warning "error here"
                 luaI_tsets(L, rfiles_idx, content.old->c, content.current->c);

                 str_free(content.old);
                 content.old = NULL;
                 str_clear(content.current);
               } else if(buffer[i] != '\r' && !(buffer[i] == ' ' && content.current->len == 0)) str_pushl(content.current, buffer + i, 1);
             }
           } 

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
             }

           }
  }

  *_content = content;

  return 0;
}
