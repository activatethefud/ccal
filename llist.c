#include "llist.h"

int compare(comparison_t *c) { return (c->fptr)(c->a,c->b); }

void add_right(node_t **head,void *new_data,size_t data_size)
{
        node_t *new_node = malloc(sizeof *new_node);

        new_node->data = malloc(data_size);
        new_node->next = NULL;
        new_node->size = 1;

        memcpy(new_node->data,new_data,data_size);

        if((*head) == NULL) {
                (*head) = new_node;
        }
        else {
                node_t *iter = (*head);
                (*head)->size++;

                while(iter->next != NULL) {
                        iter = iter->next;
                }

                iter->next = new_node;
        }

}

void add_left(node_t **head,void *new_data,size_t data_size)
{
        node_t *new_node = malloc(sizeof *new_node);

        new_node->data = malloc(data_size);
        new_node->next = NULL;
        new_node->size = 1;

        memcpy(new_node->data,new_data,data_size);

        if((*head) == NULL) {
                (*head) = new_node;
        }
        else {
                new_node->next = (*head);
                (*head) = new_node;
                (*head)->size++;
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
        int n = (*head)->size--;

        // Delete first node
        if(index == 0) {

                if(n == 1) { free(*head); (*head) = NULL; return; }

                node_t *tmp = (*head);
                (*head)->next->size = (*head)->size;
                (*head) = (*head)->next;
                free_node(tmp);
                return;
        }

        // Delete last node
        if(index == n-1) {
                node_t *iter = (*head);

                for(int i=0;i<n-2;++i) {
                        iter = iter->next;
                }


                free_node(iter->next);
                iter->next = NULL;

                return;
        }


        // Delete middle node
        else {

                node_t *iter = (*head);

                for(int i=0;i<index-2;++i) {
                        iter = iter->next;
                }

                node_t *tmp = iter->next;
                iter->next = iter->next->next;

                free(tmp);
                return;
        }
}

void delete_list(node_t *head)
{
        if(head == NULL) return;
        delete_list(head->next);

        free(head->data);
        free(head);
}

node_t *get_node(node_t *head,int index)
{
        assert(index < head->size);

        while(index--) {
                head=head->next;
        }

        return head;
}
