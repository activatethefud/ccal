#include "tokenizer.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#define DELIMITER ",\n"

tokenizer_t *create_tokenizer(char *_line,const char *delimiter)
{
        char *line = strdup(_line);

        tokenizer_t *tokenizer = malloc(sizeof *tokenizer);

        tokenizer->allocated_size = 1;
        tokenizer->token_count = 0;

        tokenizer->tokens = malloc(sizeof *tokenizer->tokens);

        char *token;

        while(tokenizer->token_count == 0 && (token = strtok(line,delimiter)) || (token = strtok(NULL,delimiter))) {

                if(tokenizer->token_count == tokenizer->allocated_size) {
                        tokenizer->allocated_size *= 2;
                        tokenizer->tokens = realloc(tokenizer->tokens,tokenizer->allocated_size * sizeof *tokenizer->tokens);
			assert(NULL != tokenizer->tokens);
			assert(NULL != tokenizer);
                }

                tokenizer->tokens[tokenizer->token_count++] = token;
        }

        return tokenizer;
}

char *tokenizer_get(tokenizer_t *tokenizer,int index)
{
        assert(index < tokenizer->token_count);
        return tokenizer->tokens[index];
}

void free_tokenizer(tokenizer_t *t)
{
        free(t->tokens);
        free(t);
}
