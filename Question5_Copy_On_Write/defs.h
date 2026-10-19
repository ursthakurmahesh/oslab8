#ifndef _DEFS_H_
#define _DEFS_H_

struct meminfo;
struct pde_t;
struct pte_t;

void  kref_inc(uint pa);
int   kref_dec(uint pa);
int   kref_get(uint pa);
int   get_free_pages_count(void);
int   get_total_pages_count(void);
int   getmeminfo(struct meminfo *mi);
pte_t* walkpgdir(pde_t *pgdir, const void *va, int alloc);
int   mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm);

#endif
