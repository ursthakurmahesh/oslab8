#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;
  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

// PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;

  // Question 4: Demand Paging Page Fault Handler (T_PGFLT = 14)
  case T_PGFLT: {
    uint va = rcr2(); // Address that caused page fault
    struct proc *curproc = myproc();

    // Check if fault address is inside heap:
    // 1. Below curproc->sz (within valid sbrk allocation limit)
    // 2. Above the user stack guard page (va >= 2 * PGSIZE)
    if(curproc != 0 && va < curproc->sz && va >= 2 * PGSIZE){
      char *mem = kalloc();
      if(mem == 0){
        // Out of physical memory: kill process cleanly
        cprintf("lazy alloc: out of physical memory for pid %d\n", curproc->pid);
        curproc->killed = 1;
        break;
      }
      memset(mem, 0, PGSIZE);

      // Map the newly allocated physical page with Write and User permissions
      if(mappages(curproc->pgdir, (char*)PGROUNDDOWN(va), PGSIZE, V2P(mem), PTE_W | PTE_U) < 0){
        cprintf("lazy alloc: mappages failed for pid %d\n", curproc->pid);
        kfree(mem);
        curproc->killed = 1;
        break;
      }

      // Page successfully installed; resume execution transparently!
      return;
    }

    // Invalid memory access: fault occurred outside heap or below stack
    cprintf("pid %d %s: segmentation fault / invalid page fault at va 0x%x (sz=0x%x)\n",
            curproc->pid, curproc->name, va, curproc->sz);
    curproc->killed = 1;
    break;
  }

  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, unexpected trap
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
