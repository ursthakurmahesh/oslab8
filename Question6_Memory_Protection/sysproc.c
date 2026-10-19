#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_mprotect(void)
{
  int addr, len, prot;
  if(argint(0, &addr) < 0 || argint(1, &len) < 0 || argint(2, &prot) < 0)
    return -1;
  return mprotect((void*)addr, len, prot);
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
sys_sbrk(void)
{
  int addr, n;
  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}
