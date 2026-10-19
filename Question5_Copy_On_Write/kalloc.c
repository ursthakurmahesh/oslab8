#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

void freerange(void *vstart, void *vend);
extern char end[];

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
  // Question 5: Page reference counts array indexed by physical page frame number
  uint ref[(PHYSTOP >> PGSHIFT)];
} kmem;

int free_pages_count = 0;
int total_pages_count = 0;

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
  total_pages_count = free_pages_count;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE){
    kmem.ref[V2P(p) >> PGSHIFT] = 0;
    kfree(p);
  }
}

// Question 5: Reference counting helper functions
void
kref_inc(uint pa)
{
  if(pa >= PHYSTOP)
    panic("kref_inc: pa out of range");
  if(kmem.use_lock)
    acquire(&kmem.lock);
  kmem.ref[pa >> PGSHIFT]++;
  if(kmem.use_lock)
    release(&kmem.lock);
}

int
kref_dec(uint pa)
{
  if(pa >= PHYSTOP)
    panic("kref_dec: pa out of range");
  int count;
  if(kmem.use_lock)
    acquire(&kmem.lock);
  if(kmem.ref[pa >> PGSHIFT] > 0)
    kmem.ref[pa >> PGSHIFT]--;
  count = kmem.ref[pa >> PGSHIFT];
  if(kmem.use_lock)
    release(&kmem.lock);
  return count;
}

int
kref_get(uint pa)
{
  if(pa >= PHYSTOP)
    panic("kref_get: pa out of range");
  int count;
  if(kmem.use_lock)
    acquire(&kmem.lock);
  count = kmem.ref[pa >> PGSHIFT];
  if(kmem.use_lock)
    release(&kmem.lock);
  return count;
}

// Free physical page only if reference count drops to 0
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");

  uint pa = V2P(v);
  if(kmem.use_lock)
    acquire(&kmem.lock);

  // If page still has active references, do not reclaim it to freelist yet
  if(kmem.ref[pa >> PGSHIFT] > 0)
    kmem.ref[pa >> PGSHIFT]--;

  if(kmem.ref[pa >> PGSHIFT] == 0){
    memset(v, 1, PGSIZE);
    r = (struct run*)v;
    r->next = kmem.freelist;
    kmem.freelist = r;
    free_pages_count++;
  }

  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4KB page and initialize reference count to 1
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);

  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    free_pages_count--;
    uint pa = V2P((char*)r);
    kmem.ref[pa >> PGSHIFT] = 1;
  }

  if(kmem.use_lock)
    release(&kmem.lock);

  return (char*)r;
}

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
