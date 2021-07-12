#include <stddef.h>

#ifndef LLIST_H
#define LLIST_H

typedef struct _node_t {
        void *data;
	size_t data_size;
        struct _node_t *next;
} node_t;

typedef struct {
        void *a;
        void *b;
        int (*fptr)(void*,void*);
} comparison_t;



extern void add_right(node_t **head,void *new_data,size_t data_size);
extern void print_list(node_t *head,void (*fptr)(void*));
extern node_t *find_node(node_t *head,comparison_t *c,void *x);
extern int find_node_index(node_t *head,comparison_t *c,void *x);
extern void free_node(node_t *head);
extern int list_len(node_t *head);
extern void delete_node(node_t **head,comparison_t *c,void *x);
extern void delete_list(node_t *head);
extern node_t *get_node_at(node_t *head,int index);
extern void add_left(node_t **head,void *new_data,size_t data_size);
extern void insert_after(node_t **head,void *data,size_t data_size,int index);
extern int list_size(node_t *head);
extern node_t *copy_list(node_t *list);
extern void sort_list(node_t *list,comparison_t *c);

#endif
