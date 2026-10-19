#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

#define MAX_SHM 8

struct shm_region {
  int key;
  char *phys_frame;
  int ref_count;
  int in_use;
};

struct {
  struct spinlock lock;
  struct shm_region regions[MAX_SHM];
} shm_table;

void
shminit(void)
{
  initlock(&shm_table.lock, "shm");
  for(int i = 0; i < MAX_SHM; i++){
    shm_table.regions[i].in_use = 0;
    shm_table.regions[i].ref_count = 0;
    shm_table.regions[i].phys_frame = 0;
  }
}

// Question 7: System call to get or allocate a shared memory region
int
shmget(int key, int size)
{
  if(size <= 0 || size > PGSIZE)
    return -1; // Supports 1-page shared regions

  acquire(&shm_table.lock);

  // Check if region with matching key already exists
  for(int i = 0; i < MAX_SHM; i++){
    if(shm_table.regions[i].in_use && shm_table.regions[i].key == key){
      release(&shm_table.lock);
      return i; // Return shmid
    }
  }

  // Find free slot
  for(int i = 0; i < MAX_SHM; i++){
    if(!shm_table.regions[i].in_use){
      char *frame = kalloc();
      if(frame == 0){
        release(&shm_table.lock);
        return -1;
      }
      memset(frame, 0, PGSIZE);
      shm_table.regions[i].key = key;
      shm_table.regions[i].phys_frame = frame;
      shm_table.regions[i].ref_count = 0;
      shm_table.regions[i].in_use = 1;
      release(&shm_table.lock);
      return i;
    }
  }

  release(&shm_table.lock);
  return -1; // Table full
}

// Attach shared frame to calling process's address space
void*
shmat(int shmid)
{
  if(shmid < 0 || shmid >= MAX_SHM)
    return (void*)-1;

  acquire(&shm_table.lock);
  if(!shm_table.regions[shmid].in_use){
    release(&shm_table.lock);
    return (void*)-1;
  }

  struct proc *curproc = myproc();
  uint va = PGROUNDUP(curproc->sz);

  // Map shared physical frame into calling process's page table
  if(mappages(curproc->pgdir, (char*)va, PGSIZE, V2P(shm_table.regions[shmid].phys_frame), PTE_W | PTE_U) < 0){
    release(&shm_table.lock);
    return (void*)-1;
  }

  curproc->sz = va + PGSIZE;
  shm_table.regions[shmid].ref_count++;
  lcr3(rcr3()); // Flush TLB

  release(&shm_table.lock);
  return (void*)va;
}
