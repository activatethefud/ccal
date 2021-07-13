#include "llist.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

int compare(comparison_t *c) { return (c->fptr)(c->a,c->b); }

int list_size(node_t *head)
{
	int size = 0;
	while(head != NULL) {
		++size;
		head = head->next;
	}
	return size;
}

node_t *create_node(void *new_data,size_t data_size)
{
	node_t *node = malloc(sizeof *node);

	node->data = malloc(data_size);
	node->next = NULL;
	node->data_size = data_size;

	memcpy(node->data,new_data,data_size);
	return node;
}

void add_right(node_t **head,void *new_data,size_t data_size)
{
        node_t *new_node = create_node(new_data,data_size);

        if((*head) == NULL) {
                (*head) = new_node;
        }
        else {
                node_t *iter = (*head);

                while(iter->next != NULL) {
                        iter = iter->next;
                }

                iter->next = new_node;
        }

}

void add_left(node_t **head,void *new_data,size_t data_size)
{
        node_t *new_node = create_node(new_data,data_size);

        if((*head) == NULL) {
                (*head) = new_node;
        }
        else {
                new_node->next = (*head);
                (*head) = new_node;
        }
}

void print_list(node_t *head,void (*fptr)(void*))
{
        while(head != NULL) {
                (*fptr)(head->data);
                head = head->next;
        }
        printf("\n");
}

node_t *find_node(node_t *head,comparison_t *c,void *x)
{
        node_t *iter = head;

        while(iter != NULL) {
                c->a = iter->data;
                c->b = x;

                if(compare(c) == 0) {
                        return iter;
                }

                iter = iter->next;
        }

        return NULL;
}

int find_node_index(node_t *head,comparison_t *c,void *x)
{
        node_t *iter = head;

        for(int index = 0;iter != NULL;iter = iter->next, ++index) {
                c->a = iter->data;
                c->b = x;

                if(compare(c) == 0) {
                        return index;
                }
        }

        return -1;
}

void free_node(node_t *head)
{
        free(head->data);
        free(head);
}

int list_len(node_t *head)
{
        int n = 0;
        while(head != NULL) {
                head = head->next;
                ++n;
        }

        return n;
}

void delete_node(node_t **head,comparison_t *c,void *x)
{
        int index = find_node_index((*head),c,x);

        // Abort if element not found
        if(index == -1) {
                return;
        }

        //int n = list_len((*head));
	int n = list_size(*head);

        // Delete first node
        if(index == 0) {

                if(n == 1) { free(*head); (*head) = NULL; return; }

                node_t *tmp = (*head);
                (*head) = (*head)->next;
                free(tmp);
        }

        else {
                node_t *prev = (*head);

                for(int i=0;i<index-1;++i) {
                        prev = prev->next;
                }

                node_t *tmp = prev->next;
                prev->next = prev->next->next;
                free(tmp);
        }

}

void delete_list(node_t *head)
{
        if(head == NULL) return;
        delete_list(head->next);

        free(head->data);
        free(head);
}

node_t *get_node_at(node_t *head,int index)
{
	assert(NULL != head);
        assert(index < list_size(head));

        while(index--) {
                head=head->next;
        }

        return head;
}

void insert_after(node_t **head,void *data,size_t data_size,int index)
{
        //int n = (*head)->size;
	int n = list_size(*head);
        assert(index < n);

        if(index == n-1) {
                add_right(head,data,data_size);
                return;
        }

        node_t *new_node = create_node(data,data_size);

        node_t *iter = (*head);
        for(int i=0;i<index;++i) {
                iter = iter->next;
        }

        new_node->next = iter->next;
        iter->next = new_node;

}

node_t *copy_list(node_t *list)
{
	node_t *copy = NULL;

	while(list != NULL) {
		add_right(&copy,list->data,list->data_size);
		list = list->next;
	}

	return copy;
}

void sort_list(node_t *list,comparison_t *c)
{
        bool swapped = true;

        if(list == NULL) return;

        while(swapped) {

                node_t *iter = list;
                swapped = false;

                while(iter->next != NULL) {
                        c->a = iter->data;
                        c->b = iter->next->data;

                        if(compare(c) > 0) {
                                void *tmp = iter->data;
                                iter->data = iter->next->data;
                                iter->next->data = tmp;
                                swapped = true;
                        }
                        
                        iter = iter->next;
                }
        }
}
