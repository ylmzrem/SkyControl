# ✈️ SkyControl — Dual Runway Management Simulator

**SkyControl** is a command-line, time-step-driven airport runway simulator written in C. It manages two runways (**Runway Alpha** and **Runway Bravo**) under three distinct operational modes, integrating **Queue**, **Stack**, and **Binary Search Tree (BST)** data structures for a realistic air traffic control simulation.

---

## 🎯 Project Objective

Build a robust, menu-driven simulation tool in C that efficiently manages dual-runway airport operations, supporting emergency protocols, congestion detection, undo functionality, and a searchable flight archive—all with strict memory management and clean modular code.

---

## 🚦 Operational Modes

At each time step, the control tower dynamically resolves the airport's operational mode based on queue conditions:

| Mode | Trigger Condition | 🛬 Runway Alpha | 🛫 Runway Bravo |
| :--- | :--- | :--- | :--- |
| **🟢 STANDARD** | Default operating state | Landings only *(Priority: Emergency > Low Fuel)* | Takeoffs only *(Pure FIFO)* |
| **🚨 EMERGENCY** | Any flight with `emergency` flag in Landing Queue | Emergency landings only *(Idle if none pending)* | Takeoffs only *(Continues normally)* |
| **⚠️ CONGESTION** | Combined queue size exceeds threshold *(Default: `5`)* | Dynamic *(Favors landing queue when larger)* | Dynamic *(Fills idle capacity)* |

---

## 🧠 Data Structure Implementations

### 1. 📋 Priority & FIFO Queues (`flight_queue.c/.h`)
Two separate queues are maintained using singly-linked `Flight` nodes:
* **Two-Tier Priority Landing Queue:**
  * **Tier 1 (Emergency):** `MAYDAY` flights are inserted ahead of all non-emergency flights.
  * **Tier 2 (Fuel Urgency):** Non-emergency flights are sorted by ascending fuel values *(lower fuel = higher priority)*.
* **Pure FIFO Takeoff Queue:** Departures are processed strictly in First-In, First-Out order.
* **⛽ Fuel Decay Mechanism:** Before each time step, `update_fuel_and_reorder_landings` decrements fuel by `1` for every waiting non-emergency flight and rebuilds the queue to reflect rising urgency.

### 2. 📚 LIFO Stack — Undo & Command History (`operation_history.c/.h`)
* Implemented as a singly-linked list of `StackNode` elements storing a full snapshot of the `Flight` struct and its assigned runway (`RWY1` or `RWY2`).
* **↩️ Rollback Support:** Triggering an undo pops the top node, deletes the matching record from the BST archive, re-inserts the flight back into its original queue, and recalculates the emergency status.

### 3. 🌳 Binary Search Tree — Flight Archive (`flight_records.c/.h`)
* Completed flights are archived in a BST keyed lexicographically by **Flight ID**.
* **Duplicate ID Support:** Equal or greater keys are routed to the right subtree, allowing multiple historical entries for the same flight number.
* **Core BST Operations:**
  * `bst_insert`: Inserts completed flight records while preserving duplicate IDs.
  * `bst_search_all`: Performs a full-tree traversal to find and count all occurrences of a given Flight ID.
  * `bst_inorder`: Executes a Left-Root-Right traversal to display the full archive sorted alphabetically.
  * `bst_delete`: Verifies all four flight fields (`id`, `fuel`, `is_landing`, `emergency`) before node removal (handling 0, 1, and 2-child cases) to ensure accurate undo operations.
  * `bst_free`: Recursively frees the entire tree in post-order upon program exit.

---

## 🏗️ System Architecture & File Structure

The project follows a **Domain-Driven Modular Architecture**, separating UI, simulation logic, and data structures into clean compilation units:

```text
SkyControl/
├── 📄 main.c                  # Interactive CLI menu, input validation loops & user prompts
├── 🧠 control_tower.c / .h    # Simulation brain, mode resolution & runway assignment logic
├── 📋 flight_queue.c / .h     # Priority landing queue, FIFO takeoff queue & fuel decay
├── 📚 operation_history.c / .h# LIFO stack for undo operations & command history logging
└── 🌳 flight_records.c / .h   # Binary Search Tree archive for completed flights
```

---

## 🛡️ Memory Management & Design Decisions

* **🧹 Zero Heap Leaks:** Every dynamically allocated node has a strict ownership lifecycle. At shutdown, `main` drains both queues, pops all stack entries, and calls `bst_free`.
* **🔒 Single-Free Pattern:** `handle_cleared_flight` passes dequeued flight data to `stack_push` and `bst_insert` (copying values) and immediately frees the queue node once.
* **🚫 Duplicate Active Flight Prevention:** `menu_add_flight` scans both active queues upon ID entry and rejects duplicate active flights immediately, while keeping underlying enqueue functions open for seamless undo re-insertions.
* **✅ Input Validation:** All menu selections and flight parameters (`fuel: 0-100`, `type: 1/2`, `emergency: 1/0`) are validated using `do-while` loops with descriptive error messages.

---

## 💻 Compilation & Usage

### 1. Compile the Project
Use `gcc` to compile all source files into an executable:

```bash
gcc main.c control_tower.c flight_queue.c operation_history.c flight_records.c -o skycontrol
```

### 2. Run the Simulator
```bash
./skycontrol
```

### 3. Interactive Menu Options
```text
--- SkyControl Menu ---
1. Add new flight
2. Process next time step
3. Display queues & status
4. Undo last runway assignment
5. Search completed flight (archive)
6. Show full archive (sorted)
7. Show command log (stack)
0. Exit
```

---

## 🎓 Academic Context

* **Course:** Data Structures & Algorithms
* **Semester:** Spring 2025–2026
* **Developer:** İrem Yılmaz 
