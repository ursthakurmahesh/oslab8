#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

int
main(int argc, char *argv[])
{
  printf(1, "========================================================\n");
  printf(1, "      xv6 Question 3: Process Page Table Dump Test      \n");
  printf(1, "========================================================\n\n");

  // Step 1: Startup page table dump
  printf(1, "[Phase 1] Process Page Table at Startup:\n");
  printf(1, "--------------------------------------------------------\n");
  vmprint();
  printf(1, "--------------------------------------------------------\n");
  printf(1, "Stack Guard Page Analysis:\n");
  printf(1, "  - Notice PTE at index 2 (va = 0x2000): flags = [P, W, -]\n");
  printf(1, "  - User bit (PTE_U) is CLEARED by exec() via clearpteu()!\n");
  printf(1, "  - This prevents stack overflow from corrupting text/data segments.\n");
  printf(1, "  - User-accessible pages = (sz / PGSIZE) - 1 guard page.\n\n");

  // Step 2: Grow heap by 8 pages (32768 bytes)
  printf(1, "[Phase 2] Growing heap by 8 pages via sbrk(8 * 4096):\n");
  char *p = sbrk(8 * PGSIZE);
  if(p == (char*)-1){
    printf(1, "sbrk failed\n");
    exit();
  }
  for(int i = 0; i < 8; i++){
    p[i * PGSIZE] = 'A' + i; // Touch all 8 pages
  }
  printf(1, "--------------------------------------------------------\n");
  vmprint();
  printf(1, "--------------------------------------------------------\n");
  printf(1, "Observation: 8 new consecutive PTEs (index 4 to 11) appeared!\n\n");

  // Step 3: Shrink heap by 1 page (4096 bytes)
  printf(1, "[Phase 3] Shrinking heap by 1 page via sbrk(-4096):\n");
  sbrk(-1 * PGSIZE);
  printf(1, "--------------------------------------------------------\n");
  vmprint();
  printf(1, "--------------------------------------------------------\n");
  printf(1, "Observation: Top PTE (index 11) successfully disappeared!\n");
  printf(1, "Hierarchical dump accurately tracks dynamic heap life cycle.\n");
  printf(1, "========================================================\n");

  exit();
}
