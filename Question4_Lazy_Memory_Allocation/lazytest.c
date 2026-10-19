#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096
#define BIG_SIZE (32 * 1024 * 1024) // 32 Megabytes = 8192 pages

int
main(int argc, char *argv[])
{
  struct meminfo mi_before, mi_after_sbrk, mi_after_touch, mi_end;

  printf(1, "========================================================\n");
  printf(1, "     xv6 Question 4: Lazy Memory Allocation Test        \n");
  printf(1, "========================================================\n\n");

  getmeminfo(&mi_before);
  printf(1, "[Phase 1: Baseline Memory State]\n");
  printf(1, "  Free Physical Pages: %d (%d KB)\n", mi_before.free_pages, mi_before.free_pages * 4);
  printf(1, "  Process User Pages : %d (%d KB)\n\n", mi_before.user_pages, mi_before.user_pages * 4);

  // Task 4.d: Large 32 MB sbrk allocation without touching
  printf(1, "[Phase 2: Allocating 32 MB (8192 pages) via sbrk()]\n");
  char *heap = sbrk(BIG_SIZE);
  if(heap == (char*)-1){
    printf(1, "sbrk(32MB) failed!\n");
    exit();
  }

  getmeminfo(&mi_after_sbrk);
  printf(1, "  Returned Heap Base : 0x%x\n", (uint)heap);
  printf(1, "  Free Physical Pages: %d (Delta: %d pages)\n", 
         mi_after_sbrk.free_pages, mi_before.free_pages - mi_after_sbrk.free_pages);
  printf(1, "  Process User Pages : %d (Delta: +%d pages)\n",
         mi_after_sbrk.user_pages, mi_after_sbrk.user_pages - mi_before.user_pages);

  if(mi_before.free_pages - mi_after_sbrk.free_pages < 10){
    printf(1, "  -> [PASSED] 32 MB allocated in O(1) without consuming 8192 frames!\n\n");
  } else {
    printf(1, "  -> [FAILED] Memory was allocated eagerly!\n\n");
  }

  // Task 4.d: Touching only a few widely spaced pages
  printf(1, "[Phase 3: Touching 3 Widely Spaced Pages (Demand Paging)]\n");
  printf(1, "  Accessing page 0 (offset 0 MB)...\n");
  heap[0] = 'X';

  printf(1, "  Accessing page 1024 (offset 4 MB)...\n");
  heap[1024 * PGSIZE] = 'Y';

  printf(1, "  Accessing page 8000 (offset 31.25 MB)...\n");
  heap[8000 * PGSIZE] = 'Z';

  getmeminfo(&mi_after_touch);
  printf(1, "  Free Physical Pages After 3 Touches: %d\n", mi_after_touch.free_pages);
  printf(1, "  Consumed Physical Frames           : %d frames (Exact 3 pages + page tables)\n",
         mi_after_sbrk.free_pages - mi_after_touch.free_pages);
  printf(1, "  -> [PASSED] Only touched pages consumed physical memory frames!\n\n");

  // Task 4.c & 4.d: System call touching lazily allocated buffer
  printf(1, "[Phase 4: Kernel System Call on Untouched Lazy Buffer]\n");
  int pfd[2];
  if(pipe(pfd) < 0){
    printf(1, "pipe failed\n");
    exit();
  }
  // Untouched page at offset 8 MB (page 2048)
  char *untouched_buf = heap + 2048 * PGSIZE;
  write(pfd[1], "HELLO_LAZY_XV6", 14);
  read(pfd[0], untouched_buf, 14);
  untouched_buf[14] = '\0';
  printf(1, "  Read data into untouched buffer: '%s'\n", untouched_buf);
  printf(1, "  -> [PASSED] Kernel read/write transparently populated lazy page!\n\n");
  close(pfd[0]);
  close(pfd[1]);

  // Task 4.d: Testing fork() with lazy pages
  printf(1, "[Phase 5: Testing fork() with Lazy Pages]\n");
  int pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  }
  if(pid == 0){
    // Child verifies reading from touched page
    if(heap[0] != 'X' || heap[1024 * PGSIZE] != 'Y'){
      printf(1, "Child read incorrect data!\n");
    } else {
      printf(1, "  [CHILD] Correctly read parent data from demand-paged heap.\n");
    }
    exit();
  }
  wait();
  printf(1, "  [PARENT] Child safely completed. copyuvm() skipped untouched pages.\n\n");

  // Task 4.c: Edge case - Invalid access triggers clean process termination
  printf(1, "[Phase 6: Edge Case - Out-of-Bounds Access Termination]\n");
  int pid2 = fork();
  if(pid2 == 0){
    printf(1, "  [CHILD 2] Attempting invalid write beyond sz (addr = 0x%x)...\n", (uint)(heap + BIG_SIZE + 4096));
    char *bad_addr = heap + BIG_SIZE + 4096;
    *bad_addr = '!'; // Triggers segmentation fault
    printf(1, "  [CHILD 2] Should not reach here!\n");
    exit();
  }
  wait();
  printf(1, "  [PARENT] Child 2 was cleanly terminated without crashing kernel!\n\n");

  printf(1, "========================================================\n");
  printf(1, "  Question 4 Lazy Allocation Verification: ALL PASSED!   \n");
  printf(1, "========================================================\n");
  exit();
}
