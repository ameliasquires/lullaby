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
    content.status = content.boundary != NULL?BARRIER_START:NORMAL;
    content.current = str_init("");
    content.line = str_init("");
    parray_clear(par, STR);

  }

  if(content.status == NORMAL){
    lua_pushvalue(L, *body_idx);
    lua_pushlstring(L, buffer, blen);
    lua_concat(L, 2);
    *body_idx = lua_gettop(L);
  } else {
file_start:;
    if(content.status == BARRIER_START){
      for(; blen > 0; buffer++, blen--){
        if(*buffer == '\n'){
          content.status = BARRIER_END;
          blen--;
          buffer++;
          break;
        }
      }
    }

    if(content.status == BARRIER_END){
      int check = 80;
      if(content.current->len < check) check = content.current->len;
      ssize_t backtrack = content.current->len - check;

      str_pushl(content.current, buffer, blen); 
      char* end = memmem(content.current->c + backtrack, content.current->len - backtrack, content.boundary->c, content.boundary->len);

      if(end != NULL){
        ssize_t size = content.current->len - (end - content.current->c + 4);
        str_popb(content.current, content.current->len - (end - content.current->c) + 4); 
        size_t header_len = (char*)memmem(content.current->c, content.current->len, "\r\n\r\n", 4) - content.current->c;

        parray_t* header = parray_init();
        parse_header_kv(content.current->c, header_len, header);

        str* cd = (str*)parray_get(header, "content-disposition");
        if(cd != NULL){
          lua_newtable(L);
          int newfile_idx = lua_gettop(L);

          parray_t* cd_parse = parray_init();
          gen_parse(cd->c, cd->len, &cd_parse);
          lua_newtable(L);
          int cdidx = lua_gettop(L);
          luaI_fromparray(L, cdidx, cd_parse, 1);

          luaI_fromparray(L, newfile_idx, header, 1);
          luaI_tsetv(L, newfile_idx, "content-disposition", cdidx);

          str* name = (str*)parray_get(cd_parse, "name");
          if(name == NULL) name = (str*)parray_get(cd_parse, "filename");

          if(name == NULL){
            int ind = lua_objlen(L, *files_idx) + 1;
            lua_pushinteger(L, ind);
            lua_pushvalue(L, newfile_idx);
            lua_settable(L, *files_idx);
          }
          else {
            luaI_tsetv(L, *files_idx, name->c, newfile_idx);
          }
          parray_clear(cd_parse, STR);

          lua_pushlstring(L, content.current->c + header_len + 4, content.current->len - header_len - 4);
          lua_setfield(L, newfile_idx, "body");
        }

        str_clear(content.current);
        parray_clear(header, STR);
        content.status = BARRIER_START;

        ssize_t oblen = blen;
        blen = oblen - (oblen - size);
        buffer += oblen - blen;
        goto file_start;
      }
    }
  }

  *_content = content;

  return 0;
}
