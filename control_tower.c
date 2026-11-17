#include "control_tower.h"

SystemMode determine_mode(Queue *land, Queue *take, int emergency_active) {
  if (emergency_active)
    return MODE_EMERGENCY;
  if (queue_total(land, take) > CONGESTION_THRESHOLD)
    return MODE_CONGESTION;
  return MODE_STANDARD;
}

static const char *mode_name(SystemMode m) {
  switch (m) {
  case MODE_STANDARD:
    return "STANDARD";
  case MODE_EMERGENCY:
    return "EMERGENCY";
  case MODE_CONGESTION:
    return "CONGESTION";
  }
  return "?";
}

void print_status(Queue *land, Queue *take, SystemMode mode, int step) {
  printf("\n--- STATUS (Step %d, Mode: %s) ---\n", step, mode_name(mode));
  printf("  Runway Alpha  -> %s\n", mode == MODE_STANDARD ? "LANDINGS"
                                    : mode == MODE_EMERGENCY
                                        ? "EMERGENCY LANDINGS ONLY"
                                        : "BOTH (congestion)");
  printf("  Runway Bravo  -> %s\n", mode == MODE_CONGESTION ? "BOTH (congestion)" : "TAKEOFFS");
  printf("--------------------------------------------------\n");
  display_queue(land, "Landing Queue ");
  display_queue(take, "Takeoff Queue ");
}

// A flight has been cleared. Process it.
static void handle_cleared_flight(Flight *f, int rwy_num, const char *rwy_label,
                                  Queue *land_q, Stack *undo, BSTNode **archive,
                                  int *emergency_active) {
  if (!f)
    return;

  printf("  [FLIGHT]  %s (%s) -> %s cleared\n", f->id,
         f->is_landing ? "LANDING" : "TAKEOFF", rwy_label);
  stack_push(undo, f, rwy_num);
  *archive = bst_insert(*archive, f);

  // If the cleared flight was an emergency, check if any other emergencies remain.
  if (f->emergency) {
    *emergency_active = has_emergency_in_queue(land_q);
  }

  free(f);
}

void process_time_step(Queue *land, Queue *take, Stack *undo, BSTNode **archive,
                       int *emergency_active, int *step) {
  SystemMode mode = determine_mode(land, take, *emergency_active);

  if (land->size > 0) {
    update_fuel_and_reorder_landings(land);
  }

  (*step)++;

  printf("\n---  Processing Time Step %d  [Mode: %s]  ---\n", *step, mode_name(mode));

  if (mode == MODE_STANDARD) {
    handle_cleared_flight(dequeue(land), 1, "Runway Alpha (landing)", land,
                          undo, archive, emergency_active);
    handle_cleared_flight(dequeue(take), 2, "Runway Bravo (takeoff)", land,
                          undo, archive, emergency_active);
  } else if (mode == MODE_EMERGENCY) {
    if (land->head && land->head->emergency) {
      handle_cleared_flight(dequeue(land), 1, "Runway Alpha (EMERGENCY)", land,
                            undo, archive, emergency_active);
    } else {
      printf("  [INFO]  No emergency landing in queue. Runway Alpha idle.\n");
    }
    handle_cleared_flight(dequeue(take), 2, "Runway Bravo (takeoff)", land,
                          undo, archive, emergency_active);
  } else {                                     // CONGESTION 
    for (int i = 1; i <= 2; i++) {
      if (land->size > take->size) {
        handle_cleared_flight(dequeue(land), i,
                              i == 1 ? "Runway Alpha (congestion-landing)"
                                     : "Runway Bravo (congestion-landing)",
                              land, undo, archive, emergency_active);
      } else if (take->size > 0) {
        handle_cleared_flight(dequeue(take), i,
                              i == 1 ? "Runway Alpha (congestion-takeoff)"
                                     : "Runway Bravo (congestion-takeoff)",
                              land, undo, archive, emergency_active);
      } else if (land->size > 0) {
        handle_cleared_flight(dequeue(land), i,
                              i == 1 ? "Runway Alpha (congestion-landing)"
                                     : "Runway Bravo (congestion-landing)",
                              land, undo, archive, emergency_active);
      }
    }
  }

  if (land->size == 0 && take->size == 0 && !(*emergency_active))
    printf("  [*]  All queues cleared!\n");
}