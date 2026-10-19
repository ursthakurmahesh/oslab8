#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

extern int shmget(int key, int size);
extern void* shmat(int shmid);

int
sys_shmget(void)
{
  int key, size;
  if(argint(0, &key) < 0 || argint(1, &size) < 0)
    return -1;
  return shmget(key, size);
}

int
sys_shmat(void)
{
  int shmid;
  if(argint(0, &shmid) < 0)
    return -1;
  return (int)shmat(shmid);
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
