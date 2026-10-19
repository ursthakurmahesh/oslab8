#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

extern int sys_fork(void);
extern int sys_exit(void);
extern int sys_wait(void);
extern int sys_sbrk(void);
extern int sys_getpid(void);
extern int sys_getmeminfo(void);

static int (*syscalls[])(void) = {
[SYS_fork]        sys_fork,
[SYS_exit]        sys_exit,
[SYS_wait]        sys_wait,
[SYS_sbrk]        sys_sbrk,
[SYS_getpid]      sys_getpid,
[SYS_getmeminfo]  sys_getmeminfo,
};

void
syscall(void)
{
  int num;
  struct proc *curproc = myproc();
  num = curproc->tf->eax;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    curproc->tf->eax = syscalls[num]();
  } else {
    curproc->tf->eax = -1;
  }
}
