#include "control_tower.h"

static void flush_input(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ;
}

static void menu_add_flight(Queue *land, Queue *take, int *emergency_active) {
  Flight f;
  memset(&f, 0, sizeof(f));

  printf("\n  Flight ID (e.g. TK001): ");
  scanf("%15s", f.id);
  flush_input();

  // Duplicate check: reject if this ID already exists in either active queue
  {
    Flight *cur = land->head;
    while (cur) {
      if (strcmp(cur->id, f.id) == 0) {
        printf("  [!]  Flight %s is already in the Landing Queue. Duplicate IDs not allowed.\n", f.id);
        return;
      }
      cur = cur->next;
    }
    cur = take->head;
    while (cur) {
      if (strcmp(cur->id, f.id) == 0) {
        printf("  [!]  Flight %s is already in the Takeoff Queue. Duplicate IDs not allowed.\n", f.id);
        return;
      }
      cur = cur->next;
    }
  }

  int type;
  do {
    printf("  Type  [1=Landing / 2=Takeoff]: ");
    if (scanf("%d", &type) != 1) {
      flush_input();
      type = 0;
      continue;
    }
    flush_input();
    if (type != 1 && type != 2)
      printf("  [!] Invalid input. Please enter 1 or 2.\n");
  } while (type != 1 && type != 2);
  f.is_landing = (type == 1);

  do {
    printf("  Fuel level (0-100): ");
    if (scanf("%d", &f.fuel) != 1) {
      flush_input();
      f.fuel = -1;
      continue;
    }
    flush_input();
    if (f.fuel < 0 || f.fuel > 100)
      printf("  [!] Fuel must be between 0 and 100.\n");
  } while (f.fuel < 0 || f.fuel > 100);

  int emg;
  do {
    printf("  Emergency? [1=Yes / 0=No]: ");
    if (scanf("%d", &emg) != 1) {
      flush_input();
      emg = -1;
      continue;
    }
    flush_input();
    if (emg != 0 && emg != 1)
      printf("  [!] Invalid input. Please enter 0 or 1.\n");
  } while (emg != 0 && emg != 1);
  f.emergency = emg;

  if (f.emergency) {
    *emergency_active = 1;
    printf("  [!]  MAYDAY declared for %s - emergency mode activated!\n", f.id);
  }

  if (f.is_landing)
    enqueue_landing(land, &f);
  else
    enqueue_takeoff(take, &f);

  printf("  [OK]  Flight %s added to %s queue.\n", f.id, f.is_landing ? "Landing" : "Takeoff");
}

static void menu_undo(Stack *undo, Queue *land, Queue *take, BSTNode **archive) {
  Flight f;
  int runway;
  if (!stack_pop(undo, &f, &runway)) {
    printf("  [X]  Nothing to undo.\n");
    return;
  }
  printf("  [UNDO]  Undo: %s removed from Runway %d, returned to queue.\n", f.id, runway);
  // Remove from archive BST as well
  *archive = bst_delete(*archive, &f);
  if (f.is_landing)
    enqueue_landing(land, &f);
  else
    enqueue_takeoff(take, &f);
}

static void menu_search(BSTNode *archive) {
  char id[MAX_ID_LEN];
  printf("  Flight ID to search: ");
  scanf("%15s", id);
  flush_input();

  int found_count = 0;
  bst_search_all(archive, id, &found_count);
  if (found_count == 0) {
    printf("  [X]  Flight %s not found in archive.\n", id);
  }
}

static void print_menu(void) {
  printf("\n--- SkyControl Menu ---\n");
  printf("1. Add new flight\n");
  printf("2. Process next time step\n");
  printf("3. Display queues & status\n");
  printf("4. Undo last runway assignment\n");
  printf("5. Search completed flight (archive)\n");
  printf("6. Show full archive (sorted)\n");
  printf("7. Show command log (stack)\n");
  printf("0. Exit\n");
  printf("-----------------------\n");
  printf("Choice: ");
}

int main(void) {
  Queue land_queue, take_queue;
  Stack undo_stack;
  BSTNode *archive = NULL;
  int emergency_active = 0;
  int step = 0;

  queue_init(&land_queue);
  queue_init(&take_queue);
  stack_init(&undo_stack);

  printf("\n");
  printf("--- SkyControl - Dual Runway Simulator ---\n");
  printf("Congestion threshold: %d flights\n", CONGESTION_THRESHOLD);

  int choice;
  do {
    print_menu();
    if (scanf("%d", &choice) != 1) {
      flush_input();
      continue;
    }
    flush_input();

    SystemMode mode =
        determine_mode(&land_queue, &take_queue, emergency_active);

    switch (choice) {
    case 1:
      menu_add_flight(&land_queue, &take_queue, &emergency_active);
      break;

    case 2:
      process_time_step(&land_queue, &take_queue, &undo_stack, &archive, &emergency_active, &step);
      break;

    case 3:
      print_status(&land_queue, &take_queue, mode, step);
      break;

    case 4:
      menu_undo(&undo_stack, &land_queue, &take_queue, &archive);
      emergency_active = has_emergency_in_queue(&land_queue);
      break;

    case 5:
      menu_search(archive);
      break;

    case 6:
      printf("\n  -- Flight Archive (sorted by ID) --\n");
      if (!archive)
        printf("    (empty)\n");
      else
        bst_inorder(archive);
      break;

    case 7:
      printf("\n  -- Command Log (last->first) --\n");
      display_stack(&undo_stack);
      break;

    case 0:
      printf("\n  Shutting down SkyControl. Goodbye!\n");
      break;

    default:
      printf("  Invalid choice.\n");
    }

  } while (choice != 0);

  // Cleanup
  Flight *f;
  while ((f = dequeue(&land_queue)))
    free(f);
  while ((f = dequeue(&take_queue)))
    free(f);
  Flight dummy;
  int rwy;
  while (stack_pop(&undo_stack, &dummy, &rwy));
  bst_free(archive);

  return 0;
}