#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "meminfo.h"

// Question 2 System Call: sys_getmeminfo
int
sys_getmeminfo(void)
{
  struct meminfo *mi;
  if(argptr(0, (void*)&mi, sizeof(*mi)) < 0)
    return -1;

  struct proc *curproc = myproc();
  if(curproc == 0 || curproc->pgdir == 0)
    return -1;

  mi->total_pages = get_total_pages_count();
  mi->free_pages = get_free_pages_count();
  mi->user_pages = curproc->sz / PGSIZE;
  mi->pt_pages = count_pt_pages(curproc->pgdir);

  return 0;
}
