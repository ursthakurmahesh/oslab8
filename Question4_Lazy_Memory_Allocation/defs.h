#ifndef _DEFS_H_
#define _DEFS_H_

struct buf;
struct context;
struct file;
struct inode;
struct pipe;
struct proc;
struct rtcdate;
struct spinlock;
struct sleeplock;
struct stat;
struct superblock;
struct meminfo;

int             deallocuvm(pde_t*, uint, uint);
int             mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm);
pte_t*          walkpgdir(pde_t *pgdir, const void *va, int alloc);
pde_t*          setupkvm(void);
void            freevm(pde_t*);
int             getmeminfo(struct meminfo *mi);

#endif
