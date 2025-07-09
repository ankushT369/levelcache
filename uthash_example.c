#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vendor/uthash.h"

struct my_struct {
    const char *name;
    int id;
    UT_hash_handle hh;
};

int main(int argc, char *argv[]) {
    const char *names[] = { "joe", "bob", "betty", NULL };
    struct my_struct *s, *tmp, *users = NULL;
    int id=0;

    for (id=0; names[id]; ++id) {
        s = (struct my_struct*)malloc(sizeof(*s));
        s->name = names[id];
        s->id = id;
        HASH_ADD_KEYPTR( hh, users, s->name, strlen(s->name), s );
    }

    HASH_FIND_STR( users, "betty", s);
    if (s) printf("betty's id is %d\n", s->id);

    HASH_ITER(hh, users, s, tmp) {
      HASH_DEL(users, s);
      free(s);
    }
    return 0;
}
