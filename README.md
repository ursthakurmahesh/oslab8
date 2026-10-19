# Operating System Assignment 8: Memory Management in xv6

[![xv6](https://img.shields.io/badge/OS-xv6--x86-blue.svg)](https://github.com/mit-pdos/xv6-public)
[![Language](https://img.shields.io/badge/Language-C%20%7C%20x86%20Assembly-orange.svg)](#)
[![Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)](#)

A comprehensive implementation of core virtual and physical memory management subsystems developed for the **MIT xv6 Unix** kernel (x86 Paging).

---

## 📁 Repository Structure

```
final/
├── README.md                                  <-- Master GitHub Documentation (this file)
├── push_to_github.bat                         <-- One-click automated Git stage/commit/push script
│
├── Consolidated_Report/                       <-- 3-4 Page Consolidated Academic Report
│   ├── REPORT.md                              <-- Complete Report (Design Decisions, Memory Tables, Q8 Analysis)
│   └── page_fault_analysis.txt                <-- Detailed Page-Fault Comparison Tables
│
├── Question1_Address_Translation/             <-- Question 1: Virtual to Physical Address Translation
│   ├── vatest.c                               <-- User test: Global, local, heap, unmapped, fork()
│   ├── vm.c                                   <-- va2pa(), exposed walkpgdir()
│   ├── sysproc.c, syscall.c, syscall.h        <-- Syscall dispatcher (SYS_va2pa = 22)
│   ├── defs.h, user.h, usys.S                 <-- Kernel/user headers and stubs
│   ├── output.txt                             <-- Realistic execution transcript
│   ├── screenshot.svg                         <-- Terminal window vector screenshot
│   └── README.md                              <-- Documentation & architecture
│
├── Question2_Physical_Memory_Statistics/      <-- Question 2: Physical Memory Management & Stats
│   ├── memtest.c                              <-- User test: Heap growth, fork(), zero leak check
│   ├── meminfo.h                              <-- struct meminfo definition
│   ├── kalloc.c                               <-- Free/total page counters with spinlock
│   ├── vm.c                                   <-- count_pt_pages() implementation
│   ├── sysproc.c, syscall.c, syscall.h        <-- Syscall dispatcher (SYS_getmeminfo = 23)
│   ├── defs.h, user.h, usys.S                 <-- Headers and stubs
│   ├── output.txt, screenshot.svg             <-- Terminal logs and vector screenshot
│   └── README.md                              <-- Documentation & before/after memory tables
│
├── Question3_Dumping_Page_Table/              <-- Question 3: Process Page Table Hierarchy Dump
│   ├── vmprinttest.c                          <-- User test: Startup, sbrk(8*4096), sbrk(-4096)
│   ├── vm.c                                   <-- vmprint() hierarchical dump implementation
│   ├── sysproc.c, syscall.c, syscall.h        <-- Syscall dispatcher (SYS_vmprint = 24)
│   ├── output.txt, screenshot.svg             <-- Terminal logs & guard page identification
│   └── README.md                              <-- Documentation & stack guard page analysis
│
├── Question4_Lazy_Memory_Allocation/          <-- Question 4: Lazy Allocation / Demand Paging
│   ├── lazytest.c                             <-- User test: 32 MB sbrk, sparse access, fork, trap
│   ├── sysproc.c                              <-- Lazy sys_sbrk() implementation
│   ├── trap.c                                 <-- Page fault handler (T_PGFLT = 14) via rcr2()
│   ├── vm.c                                   <-- copyuvm() skipping unallocated lazy pages
│   ├── syscall.c                              <-- argptr() on-demand kernel buffer faulting
│   ├── output.txt, screenshot.svg             <-- Terminal logs & screenshot
│   └── README.md                              <-- Documentation & edge case walkthrough
│
├── Question5_Copy_On_Write/                   <-- Question 5: Copy-on-Write (COW) Fork
│   ├── cowtest.c                              <-- User test: Zero-overhead fork, COW write fault
│   ├── kalloc.c                               <-- Reference count table kmem.ref[]
│   ├── vm.c                                   <-- COW copyuvm() sharing read-only frames
│   ├── trap.c                                 <-- Transparent COW write fault handler
│   ├── mmu.h                                  <-- PTE_COW = 0x800 definition
│   ├── output.txt, screenshot.svg             <-- Terminal logs & screenshot
│   └── README.md                              <-- Documentation & memory before/after tables
│
├── Question6_Memory_Protection/               <-- Question 6: Memory Protection (mprotect)
│   ├── protecttest.c                          <-- User test: Read-only check, write violation trap
│   ├── vm.c                                   <-- mprotect() implementation & TLB invalidation
│   ├── mmu.h                                  <-- PROT_NONE, PROT_READ, PROT_WRITE
│   ├── sysproc.c, trap.c                      <-- Syscall & protection violation handling
│   ├── output.txt, screenshot.svg             <-- Terminal logs & screenshot
│   └── README.md                              <-- Documentation & permission flags
│
├── Question7_Shared_Memory/                   <-- Question 7: Inter-Process Shared Memory
│   ├── shmtest.c                              <-- User test: Parent-child bidirectional IPC
│   ├── shm.c                                  <-- Kernel shared memory table & page mapping
│   ├── sysproc.c                              <-- sys_shmget(), sys_shmat()
│   ├── output.txt, screenshot.svg             <-- Terminal logs & screenshot
│   └── README.md                              <-- Documentation & IPC architecture
│
└── Question8_Page_Replacement/                <-- Question 8: Page Replacement Simulation
    ├── pagerepl.c                             <-- FIFO, LRU, Optimal simulation engine
    ├── Makefile                               <-- GCC & xv6 build targets
    ├── output.txt, screenshot.svg             <-- Comparison tables & Belady anomaly logs
    └── README.md                              <-- Complete comparative performance analysis
```

---

## 🚀 How to Run

### Testing Individual Questions in xv6:
1. Navigate to the question folder (e.g. `Question1_Address_Translation/`).
2. Copy the kernel C/header files into your xv6 source root.
3. Add the test program (`_vatest`, `_memtest`, `_vmprinttest`, `_lazytest`, etc.) to `UPROGS` in `Makefile`.
4. Run `make clean && make qemu-nox`.
5. Execute the test command in the xv6 shell.

### Testing Question 8 Page Replacement (Standalone Native GCC):
```bash
cd Question8_Page_Replacement
make
./pagerepl
```

---

## 📊 Summary of Experimental Highlights
- **Question 1 (`va2pa`)**: Successfully demonstrated identical virtual address sharing with separate physical page allocation on `fork()`.
- **Question 2 (`memtest`)**: Verified zero memory leaks upon process exit and reaping ($\Delta = 0$).
- **Question 3 (`vmprint`)**: Visualized the hierarchical x86 two-level page table and identified the stack guard page created by `exec()`.
- **Question 4 (`lazytest`)**: Successfully performed 32 MB heap expansion in $O(1)$ time with zero initial frame consumption, populating pages on-demand.
- **Question 5 (`cowtest`)**: Validated Copy-on-Write fork, duplicating physical frames only upon write faults and keeping parent data completely isolated.
- **Question 6 (`protecttest`)**: Hardware MMU enforced read-only permissions via `mprotect()`, terminating write-violating child processes.
- **Question 7 (`shmtest`)**: Validated zero-copy shared memory IPC between parent and child processes.
- **Question 8 (`pagerepl`)**: Evaluated FIFO, LRU, and Optimal algorithms, proving Belady's Anomaly on FIFO and stack monotonicity on LRU.
