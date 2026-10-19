# Question 7: Inter-Process Shared Memory (`shm`)

## Overview
Shared memory is the fastest IPC mechanism in operating systems because it eliminates kernel message buffering. By mapping identical physical frames into the virtual address spaces of two distinct processes, updates written by one process become immediately visible to the other.

---

## Architecture & Implementation
1. **`shm_table` in `shm.c`**:
   - Manages an array of shared memory descriptors identified by an integer `key`.
   - Allocates backing physical pages via `kalloc()`.
2. **`int shmget(int key, int size)`**:
   - Finds existing segment by key or allocates a new physical frame.
3. **`void* shmat(int shmid)`**:
   - Walks calling process page table, allocates page table slot at heap boundary (`sz`), and maps the backing physical frame with `PTE_U | PTE_W`.
