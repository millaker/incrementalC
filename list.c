#include "list.h"

void list_insert_head(List *head, void *val){
    List *front = head->next, *curr = head->next;
    List *insert = NEW_LIST_NODE;
    insert->next = front;
    insert->prev = head;
    insert->val = val;
    curr->prev = insert;
    head->next = insert;
}

void list_insert_tail(List *head, void *val){
    List *back = head->prev, *curr = head->prev;
    List *insert = NEW_LIST_NODE;
    insert->next = head;
    insert->prev = back;
    insert->val = val;
    curr->next = insert;
    head->prev = insert;
}

List* list_remove_head(List *head){
    List *del = head->next, *new = head->next->next;
    new->prev = head;
    head->next = new;
    return del;
}

List* list_remove_tail(List *head){
    List *del = head->prev, *new = head->prev->prev;
    new->next = head;
    head->prev = new;
    return del;
}


List *new_list(){
    List *res = NEW_LIST_NODE;
    res->next = res;
    res->prev = res;
    res->val = NULL;
    return res;
}

int list_count(List *head){
    int res = 0;
    List *curr = head->next;
    while(curr != head){
        res++;
        curr = curr->next;
    }
    return res;
}


