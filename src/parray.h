
typedef struct {
    void* value;
    str* key;
} pelem_t;

typedef struct {
    pelem_t* P;
    int len;
} parray_t;

parray_t* parray_init();

void parray_set(parray_t*, char*, void*);

void* parray_get(parray_t* , char*);

void parray_clear(parray_t*, int);