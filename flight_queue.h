#ifndef FLIGHT_QUEUE_H
#define FLIGHT_QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ID_LEN      16
#define CONGESTION_THRESHOLD 5

typedef struct Flight {
    char  id[MAX_ID_LEN];
    int   fuel;          // 0-100 */
    int   is_landing;    // 1 = landing, 0 = takeoff
    int   emergency;     // 1 = emergency, 0 = normal
    struct Flight *next;
} Flight;

typedef struct {
    Flight *head;
    Flight *tail;
    int    size;
} Queue;

void    queue_init(Queue *q);
void    enqueue_landing(Queue *q, Flight *f);
void    enqueue_takeoff(Queue *q, Flight *f);
Flight *dequeue(Queue *q);
void    display_queue(const Queue *q, const char *label);
int     queue_total(const Queue *land, const Queue *take);
int     has_emergency_in_queue(const Queue *q);
void    update_fuel_and_reorder_landings(Queue *q);

#endif