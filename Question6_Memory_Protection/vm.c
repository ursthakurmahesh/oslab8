#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

// Question 6: Memory Protection System Call Implementation
// Modifies page access permissions (Read / Write / None) for user memory range [addr, addr+len)
int
mprotect(void *addr, int len, int prot)
{
  uint va_start = (uint)addr;
  if(va_start % PGSIZE != 0 || len <= 0)
    return -1; // Must be page-aligned and positive length

  struct proc *curproc = myproc();
  uint va_end = va_start + len;

  if(va_end > curproc->sz || va_end < va_start)
    return -1; // Out of user address bounds

  for(uint va = va_start; va < va_end; va += PGSIZE){
    pte_t *pte = walkpgdir(curproc->pgdir, (const void*)va, 0);
    if(pte == 0 || !(*pte & PTE_P))
      return -1; // Page must be present

    if(prot == PROT_NONE){
      *pte &= ~(PTE_U | PTE_W);
    } else if(prot == PROT_READ){
      *pte |= PTE_U;
      *pte &= ~PTE_W;
    } else if(prot & PROT_WRITE){
      *pte |= (PTE_U | PTE_W);
    }
  }

  // Invalidate TLB caches so new permissions take effect immediately
  lcr3(rcr3());
  return 0;
}
