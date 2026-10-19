# Question 1: Virtual to Physical Address Translation

## Overview
In the x86 architecture, virtual addresses undergo hardware translation via two-level paging. A 32-bit virtual address is split as:
- **Bits [31:22]** (10 bits): Page Directory Index (`PDX`)
- **Bits [21:12]** (10 bits): Page Table Index (`PTX`)
- **Bits [11:0]**  (12 bits): Byte Offset within the 4 KB page frame

This question implements the system call `int va2pa(int va)` which exposes this translation directly to user programs while printing kernel-level translation details.

---

## System Call Specification

```c
int va2pa(int va);
```

- **Input**: 32-bit virtual address `va`.
- **Operation**:
  1. Walks the calling process's page table directory (`myproc()->pgdir`) using `walkpgdir()`.
  2. If the page table entry does not exist, or `PTE_P` (present) is not set, or `PTE_U` (user-accessible) is not set, returns `-1`.
  3. Extracts the frame physical base address: `PTE_ADDR(*pte)`.
  4. Computes physical address: `pa = PTE_ADDR(*pte) | (va & 0xFFF)`.
  5. Kernel output: Prints `pde_idx`, `pte_idx`, `offset`, and flags `[P, W, U]` via `cprintf`.
- **Return Value**: Physical address `pa` as an integer on success, or `-1` on error.

---

## Source Files in this Folder

| File | Purpose |
| :--- | :--- |
| `vm.c` | Implements `va2pa()` and exposes non-static `walkpgdir()` |
| `sysproc.c` | Syscall wrapper `sys_va2pa()` fetching user argument |
| `defs.h` | Kernel prototypes for `va2pa()` and `walkpgdir()` |
| `syscall.h` | Defines syscall number `#define SYS_va2pa 22` |
| `syscall.c` | Registers `sys_va2pa` in the syscall dispatch table |
| `user.h` | User-space function prototype `int va2pa(int va);` |
| `usys.S` | Assembly stub `SYSCALL(va2pa)` |
| `vatest.c` | Comprehensive test suite covering global, local, heap, unmapped, and fork() |
| `output.txt` | Raw terminal execution transcript |
| `screenshot.svg` | Scalable vector graphic terminal screenshot |
| `Makefile` | Build configuration instructions |

---

## Experimental Verification (`vatest.c`)

1. **Global, Local, and Heap Addresses**:
   - Global Variable `0x2ff8`: mapped to frame `0xdfde000` (PA: `0xdfdeff8`).
   - Local Stack Variable `0x3fe8`: mapped to frame `0xdfdc000` (PA: `0xdfdcfe8`).
   - Heap Block `0x4010`: mapped to frame `0xdfdb000` (PA: `0xdfdb010`).

2. **Unmapped Address Check**:
   - `va2pa(0x7FFFFFFF)` returns `-1` (no PTE allocated).
   - `va2pa(0x80000000)` returns `-1` (`PTE_U` cleared, protected kernel space).

3. **`fork()` Address Space Isolation**:
   - Parent and child share identical virtual address `0x2ff8` for `global_var`.
   - Child PA: `0xdfd8ff8` $\ne$ Parent PA: `0xdfdeff8`.
   - Proves xv6 `copyuvm()` allocates independent physical frames during fork.
