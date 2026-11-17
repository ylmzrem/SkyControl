#include "flight_queue.h"

/* QUEUE  –  FIFO with priority insertion for landing queue
  Emergency flights jump to the front.
  Among non-emergency landings, lower fuel -> closer to front.
  Takeoff queue is pure FIFO. */
void queue_init(Queue *q) {
  q->head = NULL;
  q->tail = NULL;
  q->size = 0;
}

// Allocate and copy a flight node
static Flight *make_node(Flight *f) {
  Flight *node = (Flight *)malloc(sizeof(Flight));
  if (!node) {
    fprintf(stderr, "malloc failed\n");
    exit(1);
  }
  *node = *f;
  node->next = NULL;
  return node;
}

/*enqueue_landing: inserts with priority.
  Order (highest -> lowest priority):
  1. Emergency flights  (front of queue)
  2. Non-emergency, sorted by ascending fuel (lower fuel = more urgent) */
void enqueue_landing(Queue *q, Flight *f) {
  Flight *node = make_node(f);

  // Empty queue
  if (!q->head) {
    q->head = q->tail = node;
    q->size++;
    return;
  }

  // Emergency: insert before the first non-emergency
  if (f->emergency) {
    // Find insertion point: after last emergency at head
    if (!q->head->emergency) {
      // All existing are non-emergency -> put at front
      node->next = q->head;
      q->head = node;
      q->size++;
      return;
    }
    // Walk past existing emergencies
    Flight *cur = q->head;
    while (cur->next && cur->next->emergency) {
      cur = cur->next;
    }
    node->next = cur->next;
    cur->next = node;
    if (!node->next)
      q->tail = node;
    q->size++;
    return;
  }

  // Non-emergency: insert after all emergencies, sorted by fuel ascending
  // Find the first non-emergency node
  Flight *prev = NULL, *cur = q->head;
  while (cur && cur->emergency) {
    prev = cur;
    cur = cur->next;
  }
  // Now cur points to first non-emergency (or NULL)
  while (cur && cur->fuel <= f->fuel) {
    prev = cur;
    cur = cur->next;
  }
  node->next = cur;
  if (prev)
    prev->next = node;
  else
    q->head = node;
  if (!node->next)
    q->tail = node;
  q->size++;
}

// enqueue_takeoff: pure FIFO
void enqueue_takeoff(Queue *q, Flight *f) {
  Flight *node = make_node(f);
  if (!q->tail) {
    q->head = q->tail = node;
  } else {
    q->tail->next = node;
    q->tail = node;
  }
  q->size++;
}

// Dequeue from front, returns heap-allocated copy (caller must free)
Flight *dequeue(Queue *q) {
  if (!q->head)
    return NULL;
  Flight *node = q->head;
  q->head = node->next;
  if (!q->head)
    q->tail = NULL;
  node->next = NULL;
  q->size--;
  return node;
}

void display_queue(const Queue *q, const char *label) {
  printf("  [%s | size=%d]: ", label, q->size);
  Flight *cur = q->head;
  if (!cur) {
    printf("(empty)\n");
    return;
  }
  while (cur) {
    printf("%s(fuel=%d%s) ", cur->id, cur->fuel, cur->emergency ? ",EMG" : "");
    cur = cur->next;
  }
  printf("\n");
}

int queue_total(const Queue *land, const Queue *take) {
  return land->size + take->size;
}

int has_emergency_in_queue(const Queue *q) {
  Flight *current = q->head;
  while (current) {
    if (current->emergency) {
      return 1;       // Emergency aircraft found
    }
    current = current->next;
  }
  return 0;   // No emergency aircraft in the queue
}

void update_fuel_and_reorder_landings(Queue *q) {
  if (!q || q->size <= 1)
    return;

  // Reduce fuel consumption (except for emergency aircraft)
  Flight *current = q->head;
  while (current) {
    if (!current->emergency && current->fuel > 0) {
      current->fuel -= 1; // Reduce fuel by 1 unit per iteration
    }
    current = current->next;
  }
  /* Reorder the queue: Recreate the entire queue with enqueue_landing. 
     We can skip emergency planes as they are already at the beginning. */
  Flight *first_non_emergency = q->head;
  while (first_non_emergency && first_non_emergency->emergency) {
    first_non_emergency = first_non_emergency->next;
  }
  // If there are only emergency aircraft or the queue is empty, there is no need for queuing
  if (!first_non_emergency || !first_non_emergency->next)
    return;

  Queue temp_q;
  queue_init(&temp_q);
  Flight *f;
  while ((f = dequeue(q))) {
    enqueue_landing(&temp_q, f);
    free(f);              // Release the node coming from dequeue
  }
  // Replace the original queue with the newly arranged queue
  *q = temp_q;
}