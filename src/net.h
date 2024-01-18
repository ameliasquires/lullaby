#include "lua.h"

int l_listen(lua_State*);

static char* http_codes[600] = {0};

#define define_http_codes()\
    http_codes[100] = "Continue ";\
    http_codes[101] = "Switching Protocols ";\
    http_codes[102] = "Processing ";\
    http_codes[103] = "Early Hints ";\
    http_codes[200] = "OK ";\
    http_codes[201] = "Created ";\
    http_codes[202] = "Accepted ";\
    http_codes[203] = "Non-Authoritative Information ";\
    http_codes[204] = "No Content ";\
    http_codes[205] = "Reset Content ";\
    http_codes[206] = "Partial Content ";\
    http_codes[207] = "Multi-Status ";\
    http_codes[208] = "Already Reported ";\
    http_codes[226] = "IM Used ";\
    http_codes[300] = "Multiple Choices ";\
    http_codes[301] = "Moved Permanently ";\
    http_codes[302] = "Found ";\
    http_codes[303] = "See Other ";\
    http_codes[304] = "Not Modified ";\
    http_codes[307] = "Temporary Redirect ";\
    http_codes[308] = "Permanent Redirect ";\
    http_codes[400] = "Bad Request ";\
    http_codes[401] = "Unauthorized ";\
    http_codes[402] = "Payment Required ";\
    http_codes[403] = "Forbidden ";\
    http_codes[404] = "Not Found ";\
    http_codes[405] = "Method Not Allowed ";\
    http_codes[406] = "Not Acceptable ";\
    http_codes[407] = "Proxy Authentication Required ";\
    http_codes[408] = "Request Timeout ";\
    http_codes[409] = "Conflict ";\
    http_codes[410] = "Gone ";\
    http_codes[411] = "Length Required ";\
    http_codes[412] = "Precondition Failed ";\
    http_codes[413] = "Content Too Large ";\
    http_codes[414] = "URI Too Long ";\
    http_codes[415] = "Unsupported Media Type ";\
    http_codes[416] = "Range Not Satisfiable ";\
    http_codes[417] = "Expectation Failed ";\
    http_codes[418] = "I'm a teapot ";\
    http_codes[421] = "Misdirected Request ";\
    http_codes[422] = "Unprocessable Content ";\
    http_codes[423] = "Locked ";\
    http_codes[424] = "Failed Dependency ";\
    http_codes[425] = "Too Early ";\
    http_codes[426] = "Upgrade Required ";\
    http_codes[428] = "Precondition Required ";\
    http_codes[429] = "Too Many Requests ";\
    http_codes[431] = "Request Header Fields Too Large ";\
    http_codes[451] = "Unavailable For Legal Reasons ";\
    http_codes[500] = "Internal Server Error ";\
    http_codes[501] = "Not Implemented ";\
    http_codes[502] = "Bad Gateway ";\
    http_codes[503] = "Service Unavailable ";\
    http_codes[504] = "Gateway Timeout ";\
    http_codes[505] = "HTTP Version Not Supported ";\
    http_codes[506] = "Variant Also Negotiates ";\
    http_codes[507] = "Insufficient Storage ";\
    http_codes[508] = "Loop Detected ";\
    http_codes[510] = "Not Extended ";\
    http_codes[511] = "Network Authentication Required ";

static const luaL_Reg net_function_list [] = {
  {"listen",l_listen},
  
  {NULL,NULL}
};
