#ifndef _DEFS_H_
#define _DEFS_H_

struct pde_t;

int   shmget(int key, int size);
void* shmat(int shmid);
void  shminit(void);
int   mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm);

#endif
