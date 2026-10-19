# Question 3: Dumping the Page Table of a Process

## Overview
Understanding the exact physical organization of an operating system's address space requires visualizing page directories and page tables. In xv6:
- Code and data live in lower virtual addresses (`0x0` to `0x2000`).
- The stack guard page is positioned directly beneath the user stack (`0x2000`), created by `exec()` with `PTE_U` explicitly cleared via `clearpteu()`.
- The user stack resides at `0x3000`.
- Dynamic heap allocations expand upwards from `0x4000`.
- Kernel mappings span `0x80000000` (KERNBASE) up to physical memory limits.

This question implements `int vmprint(void)` which prints this complete hierarchy.

---

## Formatting Specification
- **Level 1 (Page Directory Entry)**:
  `.. pde [idx]: pt_frame=0x..., flags=[P,W,U]`
- **Level 2 (Page Table Entry)**:
  `.... pte [idx]: va=0x... -> pa=0x..., flags=[P,W,U]`
- **Scope**: User-space mappings (`va < KERNBASE`), terminated with a one-line kernel entry summary.

---

## Stack Guard Page Verification
`exec.c` allocates 2 pages for the user stack and clears `PTE_U` on the bottom page:
```c
sz = PGROUNDUP(sz);
sz = allocuvm(pgdir, sz, sz + 2*PGSIZE);
clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
```
In the dump:
`.... pte [2]: va=0x2000 -> pa=0xdfdd000, flags=[P,W,-]`
`PTE_U` is disabled (`-`), making any attempted access by user instructions trigger a fatal trap 14 (page fault), effectively protecting the binary code and data from stack overflows.
