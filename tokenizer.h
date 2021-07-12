#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef struct {
        int allocated_size;
        int token_count;
        char **tokens;
} tokenizer_t;

extern tokenizer_t *create_tokenizer(char *line,const char *delimiter);
extern char *tokenizer_get(tokenizer_t *tokenizer,int index);
extern void delete_tokenizer(tokenizer_t *t);

#endif
