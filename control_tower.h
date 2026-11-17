#ifndef CONTROL_TOWER_H
#define CONTROL_TOWER_H

#include "flight_queue.h"
#include "operation_history.h"
#include "flight_records.h"

typedef enum {
    MODE_STANDARD,
    MODE_EMERGENCY,
    MODE_CONGESTION
} SystemMode;

SystemMode determine_mode(Queue *land, Queue *take, int emergency_active);
void       process_time_step(Queue *land, Queue *take, Stack *undo, BSTNode **archive, int *emergency_active, int *step);
void       print_status(Queue *land, Queue *take, SystemMode mode, int step);

#endif