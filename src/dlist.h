#ifndef _DEVLIB_LIST_H
#define _DEVLIB_LIST_H

#include "common.h"
struct dlist_node
{
    struct dlist_node* prev;
    struct dlist_node* next;
};

#define INIT_DLIST_HEAD(head)   do{ (head)->prev = (head); (head)->next = (head); }while(0)
#define DLIST_EMPTY(head) ((head)->next == (head))

static inline struct dlist_node* dlist_add(struct dlist_node* head, struct dlist_node* node)
{
    struct dlist_node* next = head->next;
    node->prev = head;
    node->next = next;
    head->next = node;
    next->prev = node;
    return node;
}

static inline struct dlist_node* dlist_del(struct dlist_node* node)
{
    struct dlist_node* prev = node->prev;
    struct dlist_node* next = node->next;
    prev->next = next;
    next->prev = prev;
    return node;
}

#define dlist_rdel(node) (list_del(node)->prev)
#define dlist_append(head, node) do{list_add(head->prev, node);}while(0)
#define dlist_for_each(pos, head)  for(pos = (head)->next; pos != (head); pos = pos->next)
#define dlist_for_each_reverse(pos, head)  for(pos = head->prev; pos != head; pos = pos->prev)
#endif
