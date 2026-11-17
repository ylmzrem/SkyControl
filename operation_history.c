#include "operation_history.h"

// STACK  –  LIFO, used for undo of last runway assignment
void stack_init(Stack *s) {
  s->top = NULL;
  s->size = 0;
}

void stack_push(Stack *s, Flight *f, int runway) {
  StackNode *node = (StackNode *)malloc(sizeof(StackNode));
  if (!node) {
    fprintf(stderr, "malloc failed\n");
    exit(1);
  }
  node->flight_copy = *f;
  node->runway = runway;
  node->next = s->top;
  s->top = node;
  s->size++;
}

/* stack_pop: pops top entry.
   Returns 1 on success, 0 if empty.
   Fills *out and *runway_out with the popped data. */
int stack_pop(Stack *s, Flight *out, int *runway_out) {
  if (!s->top)
    return 0;
  StackNode *node = s->top;
  s->top = node->next;
  *out = node->flight_copy;
  *runway_out = node->runway;
  free(node);
  s->size--;
  return 1;
}

void display_stack(const Stack *s) {
  printf("  [Command Log | size=%d]: ", s->size);
  StackNode *cur = s->top;
  if (!cur) {
    printf("(empty)\n");
    return;
  }
  while (cur) {
    printf("[%s | %s | fuel=%d%s -> RWY%d] ", cur->flight_copy.id,
           cur->flight_copy.is_landing ? "LND" : "TKO", cur->flight_copy.fuel,
           cur->flight_copy.emergency ? ",EMG" : "", cur->runway);
    cur = cur->next;
  }
  printf("\n");
}