#include "common.h"
#include "../types/larray.h"

#define MIMETYPES "/etc/mime.types"
/**
 * @brief calls recv into buffer until everything is read
 *
 * buffer is allocated in BUFFER_SIZE chunks
 *
 * @param {int} fd of the connection
 * @param {char**} pointer to a unallocated buffer
 * @param {int*} pointer to an int, will be where the header ends
 * @return {int64_t} bytes read, -1 if the body was damaged, -2 if the header was
*/
int64_t recv_full_buffer(int client_fd, char** _buffer, int* header_eof, int* state);
int64_t recv_header(int client_fd, char** _buffer, char** header_eof);

/**
 * @brief converts the request buffer into a parray_t
 *
 * @param {char*} request buffer
 * @param {int} where the header ends
 * @param {parray_t**} pointer to a unallocated parray_t
 * @return {int} returns 0 or -1 on failure
*/
int parse_header(char* buffer, int header_eof, parray_t** _table);

/**
 * @brief contructs an http request
 *
 * @param {str**} pointer to an unallocated destination string
 * @param {int} response code
 * @param {char*} string representation of the response code
 * @param {char*} all other header values
 * @param {char*} response content
 * @param {size_t} content length
*/
void http_build(str** _dest, int code, const char* code_det, char* header_vs, char* content, size_t len);

/**
 * @brief gets a string representation of a http code
 *
 * @param {int} http response code
 * @param {char*} allocated destination string
*/
const char* http_code(int code);

void client_fd_errors(int client_fd);

int content_disposition(str* src, parray_t** _dest);

parray_t* route_match(parray_t* paths, char* path, larray_t** params);

int match_param(char* path, char* match, parray_t* arr);

void parse_mimetypes();

int net_error(int fd, int code);
