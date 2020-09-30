#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef LLIST_H
#define LLIST_H
#include "types.h"

void add_right(node_t **head,void *new_data,size_t data_size);
void print_list(node_t *head,void (*fptr)(void*));
node_t *find_node(node_t *head,comparison_t *c,void *x);
int find_node_index(node_t *head,comparison_t *c,void *x);
void free_node(node_t *head);
int list_len(node_t *head);
void delete_node(node_t **head,comparison_t *c,void *x);

#endif
