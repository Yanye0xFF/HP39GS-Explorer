#include "stack.h"


void push(NODE **head, void *data) {
	NODE *n = sys_chkptr(malloc(sizeof(NODE)));
	n->prev = *head;
	n->data = data;
	*head = n;
}

void *pop(NODE **head) {
	if (!*head) {
		return NULL;
	}
	NODE *n = *head;
	void *data = (*head)->data;
	*head = (*head)->prev;
	free(n);
	return data;
} 

STACK *stack_push(STACK *prev, uint32_t data) {
    STACK *tmp = (STACK *)malloc(sizeof(STACK));
    tmp->prev = prev;
    tmp->data = data;
    return tmp;
}

STACK *stack_pop(STACK *cur, uint32_t *data) {
    if(cur == NULL) {
        *data = 0;
        return NULL;
    }
    STACK *prev = cur->prev;
    *data = cur->data;
    free(cur);
    return prev; 
}

/*
@unused
uint32_t stack_peek(STACK *cur) {
    return (cur == NULL) ? 0 : cur->data;
}
*/

void stack_clear(STACK *cur) {
    uint32_t dat = 0;
    while(cur != NULL) {
        cur = stack_pop(cur, &dat);
    }
}
