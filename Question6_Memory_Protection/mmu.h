#ifndef _MMU_H_
#define _MMU_H_

#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User

// Question 6: Memory Protection Flags
#define PROT_NONE       0x0     // No access allowed
#define PROT_READ       0x1     // Read-only access
#define PROT_WRITE      0x2     // Read and Write access

#define PTE_ADDR(pte)   ((uint)(pte) & ~0xFFF)
#define PTE_FLAGS(pte)  ((uint)(pte) &  0xFFF)

#define PGSIZE          4096
#define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))

#define PDX(va)         (((uint)(va) >> 22) & 0x3FF)
#define PTX(va)         (((uint)(va) >> 12) & 0x3FF)

#endif
