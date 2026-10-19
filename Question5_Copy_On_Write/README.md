# Question 5: Copy-on-Write (COW) Fork

## Overview
Traditional UNIX `fork()` eagerly copies the entire virtual address space of the parent into the child. When processes fork merely to execute `exec()`, copying pages incurs substantial CPU and memory overhead.

Copy-on-Write (COW) optimizes this by:
1. Sharing physical frames between parent and child on `fork()`.
2. Setting `PTE_COW = 0x800` and clearing `PTE_W` in both page tables.
3. Maintaining reference counts per physical frame in `kalloc.c`.
4. Deferring actual memory copying until either process attempts to write to a shared page.

---

## Reference Counting & Page Fault Handler

- **`page_ref[]` in `kalloc.c`**: Tracks how many page tables reference each frame.
- **On Write (`T_PGFLT`)**:
  - If `ref > 1`: Allocates a new frame, copies data, decrements old frame reference count, and maps new frame with `PTE_W` enabled.
  - If `ref == 1`: Clears `PTE_COW` and enables `PTE_W` in place without memory duplication.
- **`kfree()`**: Only returns the frame to the free list when `ref == 0`.
