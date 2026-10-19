#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

// Question 4: Lazy Allocation for sys_sbrk()
// When expanding (n > 0), simply increment process size (sz) without allocating
// physical frames immediately. Physical pages will be faulted in on demand.
// When shrinking (n < 0), deallocate memory pages using deallocuvm().
int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;

  struct proc *curproc = myproc();
  addr = curproc->sz;

  if(n > 0){
    // Lazy allocation: defer physical page allocation until page fault occurs
    curproc->sz += n;
  } else if(n < 0){
    // Shrinking: truly deallocate physical memory
    if(curproc->sz + n < 0)
      return -1;
    curproc->sz = deallocuvm(curproc->pgdir, addr, addr + n);
  }

  return addr;
}

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;
}

int
sys_wait(void)
{
  return wait();
}

int
sys_getpid(void)
{
  return myproc()->pid;
}
