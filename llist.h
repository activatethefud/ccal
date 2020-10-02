#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
void delete_list(node_t *head);
node_t *get_node(node_t *head,int index);
void add_left(node_t **head,void *new_data,size_t data_size);
void insert_after(node_t **head,void *data,size_t data_size,int index);

#endif
