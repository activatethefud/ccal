#include "llist.h"

void add_right(node_t **head,void *new_data,size_t data_size)
{
        node_t *new_node = malloc(sizeof *new_node);

        new_node->data = malloc(data_size);
        new_node->next = NULL;

        memcpy(new_node->data,new_data,data_size);

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
        int n = list_len((*head));

        if(n == 0) {
                return;
        }

        // Delete first node
        if(index == 0) {
                node_t *tmp = (*head);
                (*head) = (*head)->next;
                free_node(tmp);
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
