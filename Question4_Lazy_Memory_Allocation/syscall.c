#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

// Fetch the nth word-sized system call argument.
int
argint(int n, int *ip)
{
  return fetchint((myproc()->tf->esp) + 4 + 4*n, ip);
}

// Question 4: Ensure lazy pages touched by system calls (e.g. read/write/pipe buffer)
// are allocated before the kernel dereferences user pointers.
int
argptr(int n, char **pp, int size)
{
  int i;
  struct proc *curproc = myproc();

  if(argint(n, &i) < 0)
    return -1;
  if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
    return -1;

  // Demand-page any unallocated pages in range [i, i+size)
  uint va_start = PGROUNDDOWN((uint)i);
  uint va_end = PGROUNDDOWN((uint)i + size - 1);
  for(uint va = va_start; va <= va_end; va += PGSIZE){
    pte_t *pte = walkpgdir(curproc->pgdir, (void*)va, 0);
    if(pte == 0 || !(*pte & PTE_P)){
      char *mem = kalloc();
      if(mem == 0)
        return -1;
      memset(mem, 0, PGSIZE);
      if(mappages(curproc->pgdir, (char*)va, PGSIZE, V2P(mem), PTE_W | PTE_U) < 0){
        kfree(mem);
        return -1;
      }
    }
  }

  *pp = (char*)i;
  return 0;
}
