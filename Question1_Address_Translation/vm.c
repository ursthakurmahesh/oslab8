#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "elf.h"

extern char data[];  // defined by kernel.ld
pde_t *kpgdir;  // for use in scheduler()

// Set up CPU's kernel segment descriptors.
// Run once on entry on each CPU.
void
seginit(void)
{
  struct cpu *c;

  // Map "logical" addresses to virtual addresses using identity map.
  // Cannot share a CODE descriptor for both kernel and user
  // because it would have to have DPL_USR, but the CPU forbids
  // an interrupt from CPL=0 to DPL=3.
  c = &cpus[cpuid()];
  c->gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
  c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
  c->gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
  c->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
  lgdt(c->gdt, sizeof(c->gdt));
}

// Return the address of the PTE in page table pgdir
// that corresponds to virtual address va.  If alloc!=0,
// create any required page table pages.
// Exposed non-static through defs.h for Question 1.
pte_t *
walkpgdir(pde_t *pgdir, const void *va, int alloc)
{
  pde_t *pde;
  pte_t *pgtab;

  pde = &pgdir[PDX(va)];
  if(*pde & PTE_P){
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
  } else {
    if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
      return 0;
    // Make sure all entries are marked not present.
    memset(pgtab, 0, PGSIZE);
    // The permissions here are overly generous, but they can
    // be further restricted by the permissions in the page table
    // entries, if necessary.
    *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;
  }
  return &pgtab[PTX(va)];
}

// =========================================================================
// Question 1: Virtual to Physical Address Translation
// =========================================================================
// Walks the calling process's page table and translates virtual address va
// to physical address pa. Prints the 10-bit page directory index, 10-bit
// page table index, 12-bit offset, and the flag bits (P, W, U).
// Returns the physical address, or -1 if unmapped or PTE_U is not set.
int
va2pa(uint va)
{
  struct proc *curproc = myproc();
  if(curproc == 0 || curproc->pgdir == 0)
    return -1;

  pte_t *pte = walkpgdir(curproc->pgdir, (const void*)va, 0);

  // Return -1 if page table entry does not exist, page is not present (PTE_P),
  // or user-mode access is not allowed (PTE_U not set).
  if(pte == 0 || !(*pte & PTE_P) || !(*pte & PTE_U))
    return -1;

  uint pdx = PDX(va);
  uint ptx = PTX(va);
  uint offset = va & 0xFFF;
  int p = (*pte & PTE_P) ? 1 : 0;
  int w = (*pte & PTE_W) ? 1 : 0;
  int u = (*pte & PTE_U) ? 1 : 0;
  uint pa = PTE_ADDR(*pte) | offset;

  // Kernel-side verification output via cprintf
  cprintf("va2pa: va=0x%x -> pde_idx=%d, pte_idx=%d, offset=0x%x, flags=[P=%d, W=%d, U=%d] -> pa=0x%x\n",
          va, pdx, ptx, offset, p, w, u, pa);

  return (int)pa;
}
