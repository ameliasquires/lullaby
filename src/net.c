#include "net/common.h"
#include "net/util.h"
#include "net/lua.h"
#include "net/luai.h"

volatile size_t threads = 0;
void* handle_client(void *_arg){
  //printf("--\n");
  //pthread_mutex_lock(&mutex);

  int read_state = 0;
  thread_arg_struct* args = (thread_arg_struct*)_arg;
  int client_fd = args->fd;
  char* buffer;
  char dummy[2] = {0, 0};
  int header_eof = -1;
  lua_State* L = args->L;
    //sleep(1);
  //create state for this thread

  /*
  lua_State* L = luaL_newstate(); 

  luaL_openlibs(L);

  pthread_mutex_lock(&mutex);
  int old_top = lua_gettop(args->L);
  lua_getglobal(args->L, "_G");

//time_start(copy)
  luaI_deepcopy(args->L, L, SKIP_GC);

//time_end("copy", copy)
  lua_settop(args->L, old_top);
  pthread_mutex_unlock(&con_mutex);

  //l_pprint(L);
  //lua_setglobal(L, "_G");
  lua_set_global_table(L);
  pthread_mutex_unlock(&mutex);
  */
  //printf("start: %f\n",(double)(clock() - begin) / CLOCKS_PER_SEC);
  //read full request
//time_start(recv)
  int64_t bytes_received = recv_full_buffer(client_fd, &buffer, &header_eof, &read_state);
  
  /*for(int i = 0; i != header_eof; i++)
    putchar(buffer[i]);
  putchar('\n');*/
  //printf("hi %li:%i\n", bytes_received,header_eof);
  
  //ignore if header is just fucked
  if(bytes_received >= -1){
    parray_t* table;

    //checks for a valid header
    if(parse_header(buffer, header_eof, &table) != -1){

      str* sk = (str*)parray_get(table, "Path");
      str* sR = (str*)parray_get(table, "Request");
      str* sT = (str*)parray_get(table, "Content-Type");
      str* sC = (str*)parray_get(table, "Cookie");
      int some = bytes_received - header_eof - 10;
      struct file_parse* file_cont = calloc(1, sizeof * file_cont);

      lua_newtable(L);
      int files_idx = lua_gettop(L);
      lua_pushstring(L, "");
      int body_idx = lua_gettop(L);

      char portc[10] = {0};
      sprintf(portc, "%i", args->port);

      str* aa = str_init(portc);
      str_push(aa, sk->c);

      //parray_t* v = parray_find(paths, aa->c);
      larray_t* params = larray_init();
      parray_t* v = route_match(paths, aa->c, &params);

      /*for(int i = 0; i != params->len; i++){
        int id = larray_geti(params, i);
        parray_t* par = params->arr[id].value;
        printf("%i\n", i);
        for(int x = 0; x != par->len; x++){
          printf("\t%s : %s\n",par->P[x].key->c, (char*)par->P[x].value);
        }

        parray_clear(par, STR);

      }

      larray_clear(params);*/
      
      if(sT != NULL)
        rolling_file_parse(L, &files_idx, &body_idx, buffer + header_eof + 4, sT, bytes_received - header_eof - 4, file_cont);

      str_free(aa);
      if(v != NULL){
        lua_newtable(L);
        int req_idx = lua_gettop(L);
        lua_newtable(L);
        int res_idx = lua_gettop(L);
        
        //handle cookies
        //TODO: enable and test with valgrind
        if(sC != NULL){
          lua_newtable(L);
          int lcookie = lua_gettop(L);

          parray_t* cookie = parray_init();
          gen_parse(sC->c, sC->len, &cookie);
          for(int i = 0; i != cookie->len; i++){
            //printf("%s %s\n", cookie->P[i].key->c, ((str*)cookie->P[i].value)->c);
            luaI_tsetsl(L, lcookie, cookie->P[i].key->c, ((str*)cookie->P[i].value)->c, ((str*)cookie->P[i].value)->len);
          }
          luaI_tsetv(L, req_idx, "cookies", lcookie);
          parray_clear(cookie, STR);

          parray_remove(table, "Cookie", NONE);
        }
        /*
        //handle files
        if(sT != NULL && bytes_received > 0){
          int pf = file_parse(L, buffer + header_eof, sT, bytes_received - header_eof);

          if(pf >= 0){
            luaI_tsetv(L, req_idx, "files", pf);
            parray_set(table, "Body", (void*)str_init(""));
          }
        }*/
        lua_pushlightuserdata(L, file_cont);

        int ld = lua_gettop(L);
        luaI_tsetv(L, req_idx, "_data", ld);
        luaI_tsetv(L, req_idx, "files", files_idx);
        //printf("cookie and file: %f\n",(double)(clock() - begin) / CLOCKS_PER_SEC);
        //parray_set(table, "Body", (void*)str_initl(buffer + header_eof + 4, buffer_len - header_eof - 4));
        //luaI_tsetsl(L, req_idx, "Body", buffer + header_eof + 4, bytes_received - header_eof - 4);
        luaI_tsetv(L, req_idx, "Body", body_idx);
        //printf("%s\n",buffer);
        for(int i = 0; i != table->len; i+=1){
          //printf("'%s' :: '%s'\n",table[i]->c, table[i+1]->c);
          luaI_tsets(L, req_idx, table->P[i].key->c, ((str*)table->P[i].value)->c);
        }

        luaI_tsets(L, req_idx, "ip", inet_ntoa(args->cli.sin_addr));

        if(bytes_received == -1){
          client_fd = -2;
        }

        luaI_tsetb(L, req_idx, "partial", read_state == 1);
        luaI_tseti(L, req_idx, "_bytes", bytes_received - header_eof - 4);
        luaI_tseti(L, req_idx, "client_fd", client_fd);
        luaI_tsetcf(L, req_idx, "roll", l_roll);
        //luaI_tsetcf(L, req_idx, "continue", l_continue);
        
        //functions
        luaI_tsetcf(L, res_idx, "send", l_send);
        luaI_tsetcf(L, res_idx, "sendfile", l_sendfile);
        luaI_tsetcf(L, res_idx, "write", l_write);
        luaI_tsetcf(L, res_idx, "close", l_close);

        //values
        luaI_tseti(L, res_idx, "client_fd", client_fd);
        luaI_tsets(L, res_idx, "_request", sR->c);

        //header table
        lua_newtable(L);
        int header_idx = lua_gettop(L);
        luaI_tseti(L, header_idx, "Code", 200);
        luaI_tsets(L, header_idx, "Content-Type", "text/html");
        
        luaI_tsetv(L, res_idx, "header", header_idx);

        //get all function that kinda match
        parray_t* owo = (parray_t*)v;
        for(int i = 0; i != owo->len; i++){
          //though these are arrays of arrays we have to iterate *again*
          struct sarray_t* awa = (struct sarray_t*)owo->P[i].value;

          //push url params 
          lua_newtable(L);
          int new_param_idx = lua_gettop(L);

          int id = larray_geti(params, i);
          parray_t* par = params->arr[id].value;

          for(int z = 0; z != par->len; z++){
            luaI_tsets(L, new_param_idx, par->P[z].key->c, (char*)par->P[z].value);
          }
          parray_clear(par, FREE);

          luaI_tsetv(L, req_idx, "paramaters", new_param_idx);

          for(int z = 0; z != awa->len; z++){
            char* path;
            struct lchar* wowa = awa->cs[z];
            //if request is HEAD, it is valid for GET and HEAD listeners 
            if(strcmp(wowa->req, "all") == 0 || strcmp(wowa->req, sR->c) == 0 ||
                (strcmp(sR->c, "HEAD") && strcmp(wowa->req, "GET"))){
                  
                  luaL_loadbuffer(L, wowa->c, wowa->len, "fun");
                  int func = lua_gettop(L);

                  //lua_pushvalue(L, func); // push function call
                  lua_pushvalue(L, res_idx); //push methods related to dealing with the request
                  lua_pushvalue(L, req_idx); //push info about the request

                  //call the function
                  lua_call(L, 2, 0);
                 
            }
          }
        }
        larray_clear(params);
        parray_lclear(owo); //dont free the rest

        lua_pushstring(L, "client_fd");
        lua_gettable(L, res_idx);
        client_fd = luaL_checkinteger(L, -1);

      }

      void* awa;
      /*if((awa = parray_get(file_cont, "_current")) != NULL) str_free(awa);
      if((awa = parray_get(file_cont, "_boundary")) != NULL) str_free(awa);
      if((awa = parray_get(file_cont, "_boundary_id")) != NULL) str_free(awa);
      if((awa = parray_get(file_cont, "_table_idx")) != NULL) free(awa);
      if((awa = parray_get(file_cont, "_status")) != NULL) free(awa);
      if((awa = parray_get(file_cont, "_dash_count")) != NULL) free(awa);

      parray_clear(file_cont, NONE);*/
      if(file_cont->boundary != NULL) str_free(file_cont->current);
      if(file_cont->boundary != NULL) str_free(file_cont->boundary);
      if(file_cont->boundary_id != NULL) str_free(file_cont->boundary_id);

      free(file_cont);
    }
    parray_clear(table, STR);
  }
  shutdown(client_fd, 2);
  close(client_fd);
  free(args);
  free(buffer);
  lua_close(L);
  //printf("closed: %f\n",(double)(clock() - begin) / CLOCKS_PER_SEC);
  pthread_mutex_lock(&mutex);
  threads--;
  pthread_mutex_unlock(&mutex);
  //printf("out\n");
//time_end("full", full)
  return NULL;
}

int start_serv(lua_State* L, int port){
  //need these on windows for sockets (stupid)
#ifdef _WIN32
  WSADATA Data;
  WSAStartup(MAKEWORD(2, 2), &Data);
#endif

  int server_fd;
  struct sockaddr_in server_addr;

  //open the socket
  if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    p_fatal("error opening socket\n");

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);


  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&(int){1}, sizeof(int)) < 0)
    p_fatal("SO_REUSEADDR refused\n");

  //bind to port
  if(bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    p_fatal("failed to bind to port\n");

  if(listen(server_fd, max_con) < 0)
    p_fatal("failed to listen\n");

  if (pthread_mutex_init(&mutex, NULL) != 0)
    p_fatal("mutex init failed\n");

  if (pthread_mutex_init(&con_mutex, NULL) != 0)
    p_fatal("con_mutex init failed\n");

  for(;;){
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int* client_fd = malloc(sizeof(int));

    if((*client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len)) < 0)
      p_fatal("failed to accept\n");

    //open a state to call shit, should be somewhat thread safe
    thread_arg_struct* args = malloc(sizeof * args);

    args->fd = *client_fd;
    args->port = port;
    args->cli = client_addr;
    args->L = luaL_newstate();

    luaL_openlibs(args->L);

    pthread_mutex_lock(&mutex);
    int old_top = lua_gettop(L);
    lua_getglobal(L, "_G");

    //time_start(copy)
    luaI_deepcopy(L, args->L, SKIP_GC);

    //time_end("copy", copy)
    lua_settop(L, old_top);

    //l_pprint(L);
    //lua_setglobal(L, "_G");
    lua_set_global_table(args->L);
    pthread_mutex_unlock(&mutex);

    pthread_mutex_lock(&mutex);
    threads++;
    pthread_mutex_unlock(&mutex);

    //pthread_mutex_lock(&con_mutex);

    //send request to handle_client()
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, (void*)args);
    pthread_detach(thread_id);

    //double lock, wait for thread to unlock it
    //pthread_mutex_lock(&con_mutex);
     
    //handle_client((void*)args);
    free(client_fd);
  }

}

int l_req_com(lua_State* L, char* req){
  lua_pushstring(L, "port");
  lua_gettable(L, 1);
  int port = luaL_checkinteger(L, -1);

  char portc[10] = {0};
  sprintf(portc, "%i", port);//, lua_tostring(L, 2));
  str* portss = str_init(portc);
  str_push(portss, (char*)lua_tostring(L, 2));

  struct lchar* awa;
  str* uwu = str_init("");
  lua_pushvalue(L, 3);
  lua_dump(L, writer, (void*)uwu, 0);
  
  awa = malloc(sizeof * awa);
  awa->c = uwu->c;
  awa->len = uwu->len;
  strcpy(awa->req, req);
  free(uwu); //yes this *should* be str_free but awa kinda owns it now:p 

  if(paths == NULL)
    paths = parray_init();

  //please free this
  void* v_old_paths = parray_get(paths, portss->c);
  struct sarray_t* old_paths;
  if(v_old_paths == NULL){
    old_paths = malloc(sizeof * old_paths);
    old_paths->len = 0;
    old_paths->cs = malloc(sizeof old_paths->cs);
  } else old_paths = (struct sarray_t*)v_old_paths;

  old_paths->len++;
  old_paths->cs = realloc(old_paths->cs, sizeof old_paths->cs * old_paths->len);
  old_paths->cs[old_paths->len - 1] = awa;

  parray_set(paths, portss->c, (void*)old_paths);
  str_free(portss);
  return 1;
}

#define gen_reqs(T)\
int l_##T##q (lua_State* L){\
  l_req_com(L, #T);\
  return 1;\
}
gen_reqs(GET);
gen_reqs(HEAD);
gen_reqs(POST);
gen_reqs(PUT);
gen_reqs(DELETE);
gen_reqs(CONNECT);
gen_reqs(OPTIONS);
gen_reqs(TRACE);
gen_reqs(PATCH);
gen_reqs(all); //non standard lol, like expressjs 'use' keyword :3

int l_lock(lua_State* L){
  pthread_mutex_lock(&lua_mutex);
  return 0;
}

int l_unlock(lua_State* L){
  pthread_mutex_unlock(&lua_mutex);
  return 0;
}

int l_listen(lua_State* L){

  if(lua_gettop(L) != 2) {
    p_fatal("not enough args");
  }
  if(lua_type(L, 1) != LUA_TFUNCTION) {
    p_fatal("(arg:1) expected a function");
  }

  int port = luaL_checkinteger(L, 2);
  
  lua_newtable(L);
  int mt = lua_gettop(L);
  luaI_tsetcf(L, mt, "GET", l_GETq);
  luaI_tsetcf(L, mt, "HEAD", l_HEADq);
  luaI_tsetcf(L, mt, "POST", l_POSTq);
  luaI_tsetcf(L, mt, "PUT", l_PUTq);
  luaI_tsetcf(L, mt, "DELETE", l_DELETEq);
  luaI_tsetcf(L, mt, "CONNECT", l_CONNECTq);
  luaI_tsetcf(L, mt, "OPTIONS", l_OPTIONSq);
  luaI_tsetcf(L, mt, "TRACE", l_TRACEq);
  luaI_tsetcf(L, mt, "PATCH", l_PATCHq);
  luaI_tsetcf(L, mt, "all", l_allq);
  lua_pushstring(L, "lock");
  lua_pushcfunction(L, l_lock);
  lua_settable(L, -3);
  
  lua_pushstring(L, "unlock");
  lua_pushcfunction(L, l_unlock);
  lua_settable(L, -3);
  
  lua_pushstring(L, "port");
  lua_pushvalue(L, 2);
  lua_settable(L, -3);

  lua_pushvalue(L, 1); //the function
  lua_pushvalue(L, -2); //the server table

  lua_pcall(L, 1, 0, 0);

  start_serv(L, port);
  return 0;
}
