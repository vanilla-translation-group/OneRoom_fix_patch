#include "thirdparty/uthash.h"

typedef struct {
    wchar_t *item;
    UT_hash_handle hh;
} *hashset_t;

int hashset_is_member(hashset_t set, const wchar_t* item) {
    hashset_t t;
    HASH_FIND(hh, set, item, wcslen(item) * sizeof(wchar_t), t);
    return t != NULL;
}

void hashset_add(hashset_t &set, const wchar_t* item) {
    hashset_t t = (hashset_t) malloc(sizeof(*t));
    t->item = _wcsdup(item);
    HASH_ADD_KEYPTR(hh, set, t->item, wcslen(t->item) * sizeof(wchar_t), t);
}
