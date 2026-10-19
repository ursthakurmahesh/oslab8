# Question 6: Memory Protection (`mprotect`)

## Overview
Virtual memory paging enables fine-grained hardware protection bits in the page table entry (PTE). The x86 MMU enforces these bits on every instruction access.

This question implements `int mprotect(void *addr, int len, int prot)`:
- `PROT_NONE`: Clears `PTE_U` and `PTE_W`, rendering the page inaccessible to user mode.
- `PROT_READ`: Enables `PTE_U` and clears `PTE_W`, ensuring read operations succeed while write attempts fault.
- `PROT_WRITE`: Enables both `PTE_U` and `PTE_W`, allowing full read and write access.

---

## Page Fault Interception
When user instructions attempt to write to a page marked `PROT_READ`, the CPU hardware pushes error code `0x7` (User mode write to protected present page) and generates trap 14 (`T_PGFLT`). The kernel prints the faulting address and kills the violating process while keeping the operating system and parent process intact.
