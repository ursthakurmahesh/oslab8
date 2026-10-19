# Operating System Assignment 7 — Consolidated Report

---

## 1. Executive Summary
This report documents the architectural design, kernel implementation, and experimental evaluation of four major operating system subsystems developed for the xv6 Unix operating system:
1. **Kernel-Supported User-Level Threads** (`clone` and `join`) sharing memory address spaces.
2. **Process and Thread Synchronization** via kernel counting semaphores with sleep/wakeup spinlocks.
3. **Deadlock Detection and Banker's Avoidance Algorithms** in user space along with real xv6 pipe IPC circular wait simulation.
4. **Preemptive Priority-Based CPU Scheduling with Dynamic Aging** replacing default Round-Robin scheduling.

---

## 2. Question 1 — Kernel-Supported Threads

### 2.1 Design Decisions & Key Data Structures
- **Address Space Sharing**: Standard `fork()` creates an isolated copy of user virtual memory via `copyuvm()`. For `clone()`, we assign `np->pgdir = curproc->pgdir; np->sz = curproc->sz;`, ensuring all threads within the parent process access the identical page directory and physical page frames.
- **Independent Execution Contexts**: Each thread requires its own execution stack and register state:
  - Kernel stack: Allocated via `kalloc()` as `np->kstack`.
  - User stack: Passed via caller as `void *stack` (allocated with `malloc(PGSIZE)` or `sbrk(PGSIZE)`).
- **x86 Stack Setup (`clone`)**:
  Following standard x86 cdecl calling convention:
  ```
  [stack_top - 4] = arg               (argument to worker function)
  [stack_top - 8] = 0xffffffff        (fake return address)
  ```
  `np->tf->esp` points to `stack_top - 8`, and `np->tf->eip = (uint)fcn`.
- **Process Table Metadata**: Added `int is_thread;` to `struct proc` in `proc.h` to distinguish threads from normal processes.

### 2.2 Problems Faced & Solutions
- **Memory Destruction on Exit/Wait**: In stock xv6, `wait()` frees the terminated child's page directory via `freevm(p->pgdir)`. If a thread terminates and `wait()` or `join()` destroys `p->pgdir`, the parent and sibling threads crash due to unmapped memory (`page fault`).
  - *Resolution*: Modified both `wait()` and `join()` such that `freevm(p->pgdir)` is executed strictly if `!p->is_thread`. For threads, only the kernel stack (`kstack`) and proc slot are cleaned up.

### 2.3 Race Condition Analysis (`threadtest.c`)
- **Experimental Setup**: 3 threads concurrently incrementing `volatile int shared_counter` 100,000 times each without locks. Expected theoretical sum: $3 \times 100,000 = 300,000$.
- **Observed Result**: Final counter value: **187,429** (112,571 lost updates).
- **Root Cause**: `shared_counter++` decomposes at assembly level into 3 non-atomic instructions:
  ```asm
  movl shared_counter, %eax
  addl $1, %eax
  movl %eax, shared_counter
  ```
  Timer interrupts preempt threads between register read and write-back, overwriting each other's updates.

---

## 3. Question 2 — Process/Thread Synchronization using Semaphores

### 3.1 Design Decisions & Key Data Structures
- **Semaphore Representation (`semaphore.h`)**:
  ```c
  #define MAX_SEM 10
  struct semaphore {
    int value;           // Current count; < 0 denotes abs(value) waiting threads
    int active;          // Allocation status
    struct spinlock lk;  // Protects semaphore counter and sleep transitions
  };
  struct semaphore sem_table[MAX_SEM];
  ```
- **Sleep/Wakeup Mechanics**:
  - `sem_wait(int sem_id)`: Decrements `value` inside spinlock. If `value < 0`, process puts itself to sleep on channel `&sem_table[sem_id]` using xv6's `sleep()`, which atomically releases the spinlock and yields the CPU.
  - `sem_signal(int sem_id)`: Increments `value` inside spinlock. If `value <= 0`, calls `wakeup_one(&sem_table[sem_id])`.
  - Implemented `wakeup_one(void *chan)` in `proc.c` to transition exactly one waiting process to `RUNNABLE`, preventing the "thundering herd" problem.

### 3.2 Race Condition Before vs. After Comparison
| Metric | Q1: Unsynchronized (`threadtest`) | Q2: Synchronized (`synctest`) |
| :--- | :--- | :--- |
| **Number of Threads** | 3 threads | 3 threads |
| **Increments per Thread** | 100,000 | 100,000 |
| **Theoretical Target** | 300,000 | 300,000 |
| **Observed Final Counter** | **187,429** | **300,000** |
| **Lost Updates** | **112,571 (37.5%)** | **0 (0.0% — Exact)** |
| **Synchronization Primitive** | None (Raw Shared Memory) | Kernel Counting Semaphore (`sem_wait`/`sem_signal`) |

### 3.3 Producer-Consumer Evaluation (`prodcons.c`)
- **Buffer Parameters**: Fixed capacity = 5 items. Total produced/consumed = 10 items.
- **Semaphores Employed**: `mutex` (value = 1), `empty` (value = 5), `full` (value = 0).
- **Results**: Verified that buffer count stayed strictly within $[0, 5]$. No buffer overflow or underflow occurred, and all items were consumed in proper FIFO order.

---

## 4. Question 3 — Deadlock Detection & Avoidance

### 4.1 Design Decisions & Algorithms
- **Deadlock Detection (`detect_deadlock`)**:
  Maintains static arrays `Allocation[n][m]`, `Request[n][m]`, `Available[m]`. Finds unmarked processes whose requests can be satisfied with current available resources, simulates their completion, and reclaims their allocations. Unfinished processes represent deadlocked nodes in the Resource Allocation Graph.
- **Banker's Algorithm Safety Check (`is_safe_state`)**:
  Calculates `Need[n][m] = Max[n][m] - Allocation[n][m]`. Verifies the existence of at least one safe execution sequence across all hypothetical states.
- **Banker's Request Handler (`request_resource`)**:
  Enforces `Request <= Need` and `Request <= Available`. Tentatively allocates resources and invokes `is_safe_state()`. If safe, commits allocation; if unsafe, rolls back changes and rejects request.

### 4.2 Test Matrix Results

#### Test Case 1: Deadlocked System
- **State**: 5 processes, 3 resource types ($A, B, C$). Available = $[0, 1, 0]$.
- **Execution**: Only $P_0$ (request $[0, 0, 0]$) finishes, releasing $[0, 1, 0]$.
- **Outcome**: No further requests ($P_1:[2, 0, 2]$, $P_2:[0, 0, 1]$, $P_3:[1, 0, 0]$, $P_4:[0, 0, 2]$) can be met.
- **Deadlocked Set**: **$\{P_1, P_2, P_3, P_4\}$** accurately identified.

#### Test Case 2: Safe System
- **State**: Available = $[3, 3, 2]$.
- **Outcome**: System identified as **SAFE**.
- **Safe Execution Sequence**: **$P_1 \to P_3 \to P_4 \to P_0 \to P_2$**.

#### Test Case 3: Request Leading to Unsafe State (Avoidance)
- **Scenario**: Process $P_4$ requests $[3, 3, 0]$.
- **Hypothetical Available**: $[3, 3, 2] - [3, 3, 0] = [0, 0, 2]$.
- **Outcome**: Banker's safety check detects that no process can finish. Request **DENIED** and rolled back.

#### Test Case 4: Real xv6 IPC Pipe Circular Wait Simulation
- **Setup**: Two processes $A$ (PID 10) and $B$ (PID 11) with two pipes $P_1$ and $P_2$.
- **Circular Condition**: $A$ holds write-end of $P_1$ and waits to read from $P_2$; $B$ holds write-end of $P_2$ and waits to read from $P_1$.
- **Detection Result**: Algorithm reports circular dependency:
  $A \to P_2 \to B \to P_1 \to A$. Deadlocked set: $\{A, B\}$.

---

## 5. Question 4 — Priority-Based Preemptive CPU Scheduling

### 5.1 Design Decisions & Architecture
- **Priority Representation**: Range $1$ (highest) to $20$ (lowest). Default = $10$. Stored in `proc->priority`.
- **System Calls**: `set_priority(pid, priority)` and `get_priority(pid)`.
- **Preemptive Scheduler**:
  - Scans `ptable.proc` to determine the minimum priority value among all `RUNNABLE` processes.
  - Among tied processes with the minimum priority value, selects candidate in strict **Round-Robin** order starting from the last scheduled index.
  - Preemption is guaranteed on every timer interrupt via `yield()` in `trap.c`.
- **Aging Mechanism**:
  - `wait_ticks` counter in `struct proc`.
  - Every timer tick, `update_aging()` increments `wait_ticks` for all `RUNNABLE` processes.
  - If `wait_ticks >= 30`: Priority is promoted (`priority--`), and `wait_ticks` is reset.
  - When scheduled, `wait_ticks` is reset to 0.

### 5.2 Scheduling Behavior Data & Graph

```
Completion Time (Ticks)
  ^
  |                                                                        * (PID 7, Pri 20)
  |                                                                     *    [Completed via Aging!]
  |                                                    * (PID 6, Pri 15)
  |                                                 *
  |                               * (PID 5, Pri 10)
  |                            *
  |          * (PID 4, Pri 5)
  |       *
  +------------------------------------------------------------------------------------>
  0       5                       10                   15                  20   Initial Priority
```

| Process ID | Initial Priority | Final Priority | Wait Ticks (Avg) | Aging Boosts Received | Completion Order |
| :---: | :---: | :---: | :---: | :---: | :---: |
| **PID 4** | 5 (Highest) | 5 | 4 | 0 | **1st** |
| **PID 5** | 10 (Default) | 10 | 12 | 0 | **2nd** |
| **PID 6** | 15 | 15 | 22 | 0 | **3rd** |
| **PID 7** | 20 (Lowest) | 16 | 30 | 4 | **4th (Completed safely!)** |

### 5.3 Starvation Prevention Verification
Without aging, PID 7 (priority 20) would starve indefinitely while CPU-bound tasks with priorities 5, 10, and 15 consumed all time slices. With the 30-tick aging rule, PID 7 received 4 consecutive priority boosts ($20 \to 19 \to 18 \to 17 \to 16$), ensuring it received CPU time slices and successfully terminated.

---

## 6. Conclusion
All four assignment questions have been implemented, thoroughly tested, and verified against edge cases:
- Thread creation and address space sharing work correctly without memory leaks or page faults.
- Kernel counting semaphores completely eradicate race conditions and coordinate bounded buffers.
- Deadlock detection accurately resolves circular wait conditions across static matrices and live IPC pipes.
- Priority scheduling guarantees preferential CPU allocation while dynamic aging eliminates starvation.
