# Question 4: Lazy Memory Allocation (Demand Paging for the Heap)

## Overview
By default, xv6 allocates physical frames eagerly whenever a process calls `sbrk(n)`. For modern operating systems, eager allocation wastes precious physical memory frames for sparse data structures, large memory-mapped buffers, or over-provisioned heaps.

This question implements **Demand Paging (Lazy Memory Allocation)**:
1. `sys_sbrk(n)` only updates `myproc()->sz += n` without allocating any page frames.
2. When the user process accesses an untouched heap page, the x86 MMU encounters an unmapped address and fires trap 14 (`T_PGFLT`).
3. The kernel catches `T_PGFLT` in `trap.c`, reads the faulting virtual address from register `%cr2`, dynamically allocates a zeroed 4 KB frame via `kalloc()`, and maps it with `mappages()` before resuming the process.

---

## Edge Cases Handled

1. **Out-of-Bounds & Negative Offsets**:
   - Addresses `>= sz` or below the user stack (`< 2 * PGSIZE`) are detected as invalid, terminating the process with an informative segmentation fault message without crashing the kernel.
2. **Kernel Dereference of Untouched Buffers (`argptr`)**:
   - If user space passes an untouched buffer to `read()`, `write()`, or `pipe()`, `argptr()` lazily faults in the required physical frame before kernel code touches it.
3. **`copyuvm()` in `fork()`**:
   - Standard xv6 calls `panic("copyuvm: page not present")` if a page entry is not present. We modified `copyuvm()` to gracefully `continue;` past unallocated PTEs.
4. **Physical Memory Exhaustion**:
   - If `kalloc()` returns `0`, the kernel safely terminates the offending process.
