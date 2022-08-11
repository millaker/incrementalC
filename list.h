#ifndef LIST_H
#define LIST_H

#include <stdlib.h>

/* Must define a free list function */

typedef struct List{
    struct List *next;
    struct List *prev;
    void *val;
} List;

void list_insert_head(List *head, void *val);
void list_insert_tail(List *head, void *val);
List* list_remove_head(List *head);
List* list_remove_tail(List *head);
int list_count(List *head);
List *new_list();

#define NEW_LIST_NODE (malloc(sizeof(List)))
#define NEW_LIST new_list()

/* Cannot remove nodes or add nodes during traversing */
#define for_each_node_unsafe(list,ptr) for(List *ptr = list->next; ptr != list; ptr = ptr->next)
#define for_each_node_reverse_unsafe(list,ptr) for(List *ptr = list->prev; ptr != list; ptr = ptr->prev)
#define list_is_empty(l) (l->next == l)


#endif
