#ifndef _DEFS_H_
#define _DEFS_H_

struct pde_t;
struct pte_t;

int    mprotect(void *addr, int len, int prot);
pte_t* walkpgdir(pde_t *pgdir, const void *va, int alloc);

#endif
