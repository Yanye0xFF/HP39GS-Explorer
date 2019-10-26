#ifndef __STACK_H__
#define __STACK_H__

#include <hpstdlib.h>
#include <stdint.h>

typedef struct node {
    struct node *prev;
    void *data;
} NODE;

void push(NODE **head, void *data);
void *pop(NODE **head);


typedef struct stack {
    struct stack *prev;
    uint32_t data;
} STACK;

STACK *stack_push(STACK *prev, uint32_t data);
STACK *stack_pop(STACK *cur, uint32_t *data);
/*
@unused
uint32_t stack_peek(STACK *cur);
*/
void stack_clear(STACK *cur);

#endif
