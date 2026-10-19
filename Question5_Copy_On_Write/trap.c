#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

void
handle_page_fault(struct trapframe *tf)
{
  uint va = rcr2();
  struct proc *curproc = myproc();

  if(curproc == 0 || va >= curproc->sz){
    cprintf("pid %d %s: segmentation fault at 0x%x\n", curproc->pid, curproc->name, va);
    curproc->killed = 1;
    return;
  }

  pte_t *pte = walkpgdir(curproc->pgdir, (void*)va, 0);
  if(pte == 0 || !(*pte & PTE_P)){
    cprintf("pid %d %s: invalid access at 0x%x\n", curproc->pid, curproc->name, va);
    curproc->killed = 1;
    return;
  }

  // Question 5: Copy-on-Write Fault Resolution
  if(*pte & PTE_COW){
    uint pa = PTE_ADDR(*pte);
    int ref = kref_get(pa);

    if(ref > 1){
      // Multiple processes sharing this page: duplicate frame
      char *mem = kalloc();
      if(mem == 0){
        curproc->killed = 1;
        return;
      }
      memmove(mem, (char*)P2V(pa), PGSIZE);
      kref_dec(pa);

      // Re-map with PTE_W restored and PTE_COW cleared
      *pte = V2P(mem) | PTE_P | PTE_W | PTE_U;
    } else {
      // Sole owner remaining: promote directly to writable without copy
      *pte &= ~PTE_COW;
      *pte |= PTE_W;
    }

    // Flush TLB
    lcr3(rcr3());
    return;
  }

  cprintf("pid %d %s: unhandled page fault at 0x%x\n", curproc->pid, curproc->name, va);
  curproc->killed = 1;
}
