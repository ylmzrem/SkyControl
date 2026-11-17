#ifndef OPERATION_HISTORY_H
#define OPERATION_HISTORY_H

#include "flight_queue.h"

typedef struct StackNode {
    Flight         flight_copy;   // snapshot of the flight
    int            runway;        // runway it was assigned to (1 or 2)
    struct StackNode *next;
} StackNode;

typedef struct {
    StackNode *top;
    int        size;
} Stack;

void    stack_init(Stack *s);
void    stack_push(Stack *s, Flight *f, int runway);
int     stack_pop(Stack *s, Flight *out, int *runway_out);
void    display_stack(const Stack *s);

#endif