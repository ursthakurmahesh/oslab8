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

// kalloc.c
char*           kalloc(void);
void            kfree(char*);
void            kinit1(void*, void*);
void            kinit2(void*, void*);
int             get_free_pages_count(void);
int             get_total_pages_count(void);

// vm.c
int             count_pt_pages(pde_t *pgdir);

// sysproc.c / syscall
int             getmeminfo(struct meminfo *mi);

// ... standard xv6 prototypes omitted for brevity ...
#endif
