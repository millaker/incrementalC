#ifndef SYMTAB
#define SYMTAB

#include "list.h"

typedef struct symtab{
    List *table;
    struct symtab *parent;
} symtab;

typedef struct id_entry{
    char *name;
    int var_address; /* offset */
    List *arg;
    /* add type info here in the future */
} id_entry;

symtab *new_symtab();
void symtab_enter(symtab *t, char *name, int var_address, List *arg);
List *symtab_find(symtab*t, char *name);
void free_symtab(symtab* t);

#define NEW_SYMTAB new_symtab()

#endif
