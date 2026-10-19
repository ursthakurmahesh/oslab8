// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

// Question 2: Physical Memory Tracking
int free_pages_count = 0;
int total_pages_count = 0;

// Initialization code for kalloc.
// Claim first 4MB for early boot allocator.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
  // Total pages available to the kernel physical allocator
  total_pages_count = free_pages_count;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);

  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;

  // Task 2.a: Increment free physical pages count under lock
  free_pages_count++;

  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    // Task 2.a: Decrement free physical pages count under lock
    free_pages_count--;
  }

  if(kmem.use_lock)
    release(&kmem.lock);

  return (char*)r;
}

// Accessor functions exposed to system call
int
get_free_pages_count(void)
{
  int count;
  if(kmem.use_lock)
    acquire(&kmem.lock);
  count = free_pages_count;
  if(kmem.use_lock)
    release(&kmem.lock);
  return count;
}

int
get_total_pages_count(void)
{
  return total_pages_count;
}
