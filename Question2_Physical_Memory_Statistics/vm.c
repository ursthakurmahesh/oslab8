#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"

// Count total pages occupied by a process's page directory and page tables
int
count_pt_pages(pde_t *pgdir)
{
  if(pgdir == 0)
    return 0;

  // 1 page for the page directory itself
  int count = 1;

  // Each present entry in the page directory points to a 4KB page table
  for(int i = 0; i < NPDENTRIES; i++){
    if(pgdir[i] & PTE_P)
      count++;
  }
  return count;
}
