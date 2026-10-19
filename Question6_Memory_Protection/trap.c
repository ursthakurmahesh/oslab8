#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"

void
handle_trap(struct trapframe *tf)
{
  if(tf->trapno == T_PGFLT){
    uint va = rcr2();
    cprintf("pid %d %s: memory protection fault at va 0x%x (err code: 0x%x)\n",
            myproc()->pid, myproc()->name, va, tf->err);
    myproc()->killed = 1;
    exit();
  }
}
