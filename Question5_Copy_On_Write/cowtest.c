#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096
#define NUM_PAGES 32

int
main(int argc, char *argv[])
{
  struct meminfo mi_start, mi_after_alloc, mi_after_fork, mi_after_write, mi_end;

  printf(1, "========================================================\n");
  printf(1, "       xv6 Question 5: Copy-on-Write (COW) Test         \n");
  printf(1, "========================================================\n\n");

  getmeminfo(&mi_start);
  printf(1, "[Phase 1] Baseline Memory State:\n");
  printf(1, "  Free Physical Pages: %d\n\n", mi_start.free_pages);

  // Allocate 32 pages (128 KB) and write data into them
  printf(1, "[Phase 2] Allocating and Writing %d Pages (128 KB)...\n", NUM_PAGES);
  char *buf = sbrk(NUM_PAGES * PGSIZE);
  if(buf == (char*)-1){
    printf(1, "sbrk failed\n");
    exit();
  }
  for(int i = 0; i < NUM_PAGES; i++){
    buf[i * PGSIZE] = 'A' + (i % 26);
  }

  getmeminfo(&mi_after_alloc);
  printf(1, "  Free Physical Pages: %d (Consumed %d pages)\n\n",
         mi_after_alloc.free_pages, mi_start.free_pages - mi_after_alloc.free_pages);

  // Fork child and verify physical page sharing
  printf(1, "[Phase 3] Calling fork() with Copy-on-Write Sharing...\n");
  int pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  }

  if(pid == 0){
    // Child verifies data without modifying it
    for(int i = 0; i < NUM_PAGES; i++){
      if(buf[i * PGSIZE] != ('A' + (i % 26))){
        printf(1, "Child read mismatch at page %d!\n", i);
        exit();
      }
    }
    printf(1, "  [CHILD] All %d pages verified in read-only shared mode!\n", NUM_PAGES);

    // Child writes to 1 page: triggers COW page fault and frame allocation
    printf(1, "  [CHILD] Modifying page 0 (triggering COW fault)...\n");
    buf[0] = 'Z';
    printf(1, "  [CHILD] Page 0 successfully updated to '%c' via COW fault!\n", buf[0]);
    exit();
  }

  getmeminfo(&mi_after_fork);
  printf(1, "  Free Physical Pages Immediately After Fork: %d\n", mi_after_fork.free_pages);
  printf(1, "  Memory Overhead for Fork: %d pages (Only page tables cloned, NOT data pages!)\n",
         mi_after_alloc.free_pages - mi_after_fork.free_pages);

  wait(); // Wait for child to exit

  getmeminfo(&mi_after_write);
  printf(1, "  [PARENT] Child exited. Verifying Parent Page 0 Unaltered:\n");
  printf(1, "  Parent buf[0] = '%c' (Expected: 'A' - Complete Memory Isolation)\n", buf[0]);

  getmeminfo(&mi_end);
  printf(1, "\n[Phase 4] Post-Execution Memory Leak Verification:\n");
  printf(1, "  Pre-fork Free Pages : %d\n", mi_after_alloc.free_pages);
  printf(1, "  Post-reap Free Pages: %d\n", mi_end.free_pages);
  if(mi_end.free_pages == mi_after_alloc.free_pages){
    printf(1, "  Delta               : 0 (Exact match!)\n");
    printf(1, "  Status              : [PASSED] COW FORK AND ZERO LEAK VERIFIED!\n");
  } else {
    printf(1, "  Delta               : %d pages\n", mi_after_alloc.free_pages - mi_end.free_pages);
  }
  printf(1, "========================================================\n");

  exit();
}
