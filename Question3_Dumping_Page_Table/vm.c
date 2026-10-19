#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"

// Question 3: Dumping the Page Table of a Process
// Prints the hierarchical two-level page table structure of the calling process.
// Formats page directory entries (PDEs) and page table entries (PTEs) below KERNBASE,
// displaying their index, mapped virtual address, frame physical address, and flags.
// Concludes with a summary count of kernel-space mapped entries above KERNBASE.
int
vmprint(void)
{
  struct proc *curproc = myproc();
  if(curproc == 0 || curproc->pgdir == 0)
    return -1;

  cprintf("page table %p (pid %d: %s)\n", curproc->pgdir, curproc->pid, curproc->name);

  int kernel_entries = 0;
  for(int i = 0; i < NPDENTRIES; i++){
    pde_t pde = curproc->pgdir[i];
    if(pde & PTE_P){
      // Check if entry belongs to kernel space (above KERNBASE 0x80000000)
      if((i << 22) >= KERNBASE){
        kernel_entries++;
        continue;
      }

      // Print level 1: Page Directory Entry
      cprintf(".. pde [%d]: pt_frame=0x%x, flags=[%s%s%s]\n",
              i,
              PTE_ADDR(pde),
              (pde & PTE_P) ? "P" : "-",
              (pde & PTE_W) ? "W" : "-",
              (pde & PTE_U) ? "U" : "-");

      pte_t *pgtab = (pte_t*)P2V(PTE_ADDR(pde));
      // Print level 2: Valid Page Table Entries
      for(int j = 0; j < NPTENTRIES; j++){
        pte_t pte = pgtab[j];
        if(pte & PTE_P){
          uint va = (i << 22) | (j << 12);
          cprintf(".... pte [%d]: va=0x%x -> pa=0x%x, flags=[%s%s%s]%s\n",
                  j,
                  va,
                  PTE_ADDR(pte),
                  (pte & PTE_P) ? "P" : "-",
                  (pte & PTE_W) ? "W" : "-",
                  (pte & PTE_U) ? "U" : "-",
                  !(pte & PTE_U) ? " <-- [GUARD PAGE: PTE_U cleared]" : "");
        }
      }
    }
  }

  cprintf("Kernel entries: %d PDEs mapped (above KERNBASE 0x80000000)\n", kernel_entries);
  return 0;
}
