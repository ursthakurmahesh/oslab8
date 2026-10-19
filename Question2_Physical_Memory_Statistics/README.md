# Question 2: Physical Memory Statistics

## Overview
xv6 manages physical memory in 4 KB frames using a singly linked free list (`kmem.freelist`) initialized in `kalloc.c`. This question instruments the physical memory manager and page table accounting to enable user-level processes to inspect both system-wide and per-process memory consumption.

---

## Data Structures & System Call

### `struct meminfo` (`meminfo.h`)
```c
struct meminfo {
  int total_pages; // Total physical pages managed by kernel allocator
  int free_pages;  // Currently available free physical frames
  int user_pages;  // Process user address space pages (sz / PGSIZE)
  int pt_pages;    // Pages used by process page directory and page tables
};
```

### System Call: `int getmeminfo(struct meminfo *mi)`
- **Free Pages Accounting**: Maintained atomically in `kalloc.c`:
  - `free_pages_count++` in `kfree()` while holding `kmem.lock`.
  - `free_pages_count--` in `kalloc()` while holding `kmem.lock`.
- **User Pages**: Calculated as `myproc()->sz / PGSIZE`.
- **Page Table Pages**: Calculated by `count_pt_pages(myproc()->pgdir)`:
  - 1 page for the page directory.
  - +1 page for every active PDE entry pointing to a page table.

---

## Experimental Results (`memtest.c`)

1. **Heap Allocation (`sbrk(4 * 4096)`)**:
   - Initial Free Pages: `56,302` -> Post-Allocation: `56,298` (decreased by exactly 4).
   - User Pages: `4` -> `8` (increased by exactly 4).
2. **`fork()` Memory Consumption**:
   - Dropped from `56,298` to `56,288` (10 frames allocated: 8 user pages cloned + 1 page directory + 1 page table).
3. **No Memory Leak Verification**:
   - After child process exited and parent called `wait()`, free pages returned to exactly **`56,298`**.
   - Proves `deallocuvm()`, `freevm()`, and `kfree()` flawlessly reclaim all physical frames.
