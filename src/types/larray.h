#include <stdint.h>
#include <stdlib.h>

struct larray_item {
    uint64_t idx;
    void* value;
    int used;
};

typedef struct {
    struct larray_item* arr;
    size_t len, size;
} larray_t;

larray_t* larray_initl(int len);
larray_t* larray_init();
void larray_expand(larray_t** _l);
int larray_set(larray_t** _l, uint64_t idx, void* value);
int larray_geti(larray_t* l, uint64_t idx);
void* larray_get(larray_t* l, uint64_t idx);
void larray_clear(larray_t* l);

