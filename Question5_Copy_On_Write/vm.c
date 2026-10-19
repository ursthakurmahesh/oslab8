#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"

// Question 5: Copy-on-Write copyuvm()
// Instead of eagerly copying physical frames on fork(), share the parent's physical
// frames read-only with PTE_W cleared and PTE_COW set in both parent and child page tables.
pde_t*
copyuvm(pde_t *pgdir, uint sz)
{
  pde_t *d;
  pte_t *pte;
  uint pa, i, flags;

  if((d = setupkvm()) == 0)
    return 0;

  for(i = 0; i < sz; i += PGSIZE){
    if((pte = walkpgdir(pgdir, (void *) i, 0)) == 0)
      continue;
    if(!(*pte & PTE_P))
      continue;

    // Clear write permission, set COW flag on parent's PTE
    *pte &= ~PTE_W;
    *pte |= PTE_COW;

    pa = PTE_ADDR(*pte);
    flags = PTE_FLAGS(*pte);

    // Increment reference count of shared physical frame
    kref_inc(pa);

    // Map into child with identical COW read-only permissions
    if(mappages(d, (void*)i, PGSIZE, pa, flags) < 0) {
      goto bad;
    }
  }

  // Flush parent TLB so write protection immediately takes effect
  lcr3(rcr3());
  return d;

bad:
  freevm(d);
  return 0;
}
