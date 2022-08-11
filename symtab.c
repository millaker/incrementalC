#include "symtab.h"
#include "list.h"
#include <stdlib.h>
#include <string.h>

symtab *new_symtab(){
    symtab *s = (symtab*) malloc(sizeof(symtab));
    s->table = NEW_LIST;
    s->parent = NULL;
    return s;
}

void symtab_enter(symtab *t, char *name, int var_address, List *arg){
    id_entry *e = (id_entry*) malloc(sizeof(&e));
    char *tempstr = (char*) malloc(sizeof(char) *(strlen(name) + 1));
    strcpy(tempstr, name);
    e->name = tempstr;
    e->arg = arg;
    e->var_address = var_address;
    list_insert_tail(t->table, e);
}

List *symtab_find(symtab *t, char *name){
    for_each_node_unsafe(t->table, ptr){
        if(!strcmp(name, ((id_entry*)(ptr->val))->name))
            return ptr;
    }
    return NULL;
}

void free_symtab(symtab *t){
    List *curr = t->table->next;
    while(curr && curr != t->table){
        List *temp = curr->next;
        free(((id_entry*)curr->val)->name);
        free(curr->val);
        free(curr);
        curr = temp;
    }
    free(t->table->val);
    free(t->table);
    free(t);
}
