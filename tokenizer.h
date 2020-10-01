#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define DELIMITER ",\n"

#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef struct {
        int allocated_size;
        int token_count;
        char **tokens;
} tokenizer_t;

tokenizer_t *create_tokenizer(char *line);
char *tokenizer_get(tokenizer_t *tokenizer,int index);
void delete_tokenizer(tokenizer_t *t);

#endif
