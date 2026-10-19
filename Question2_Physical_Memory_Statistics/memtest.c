#include "types.h"
#include "stat.h"
#include "user.h"
#include "meminfo.h"

void print_mem_state(const char *phase, struct meminfo *mi)
{
  printf(1, "[%s]\n", phase);
  printf(1, "  Total System Pages : %d (%d MB)\n", mi->total_pages, (mi->total_pages * 4) / 1024);
  printf(1, "  Free Physical Pages: %d (%d KB)\n", mi->free_pages, mi->free_pages * 4);
  printf(1, "  Process User Pages : %d (%d KB)\n", mi->user_pages, mi->user_pages * 4);
  printf(1, "  Page Table Pages   : %d (%d KB)\n\n", mi->pt_pages, mi->pt_pages * 4);
}

int
main(int argc, char *argv[])
{
  struct meminfo mi_initial, mi_allocated, mi_fork, mi_final;

  printf(1, "========================================================\n");
  printf(1, "    xv6 Question 2: Physical Memory Statistics Test     \n");
  printf(1, "========================================================\n\n");

  // Step 1: Initial baseline memory state
  if(getmeminfo(&mi_initial) < 0){
    printf(1, "getmeminfo failed\n");
    exit();
  }
  print_mem_state("Phase 1: Initial Baseline", &mi_initial);

  // Step 2: Allocate memory via sbrk (4 pages = 16 KB)
  printf(1, "--- Growing heap by 4 pages (16384 bytes) via sbrk() ---\n");
  char *mem = sbrk(4 * 4096);
  if(mem == (char*)-1){
    printf(1, "sbrk failed\n");
    exit();
  }
  // Touch pages to ensure physical allocation
  for(int i = 0; i < 4; i++){
    mem[i * 4096] = (char)(i + 1);
  }

  if(getmeminfo(&mi_allocated) < 0){
    printf(1, "getmeminfo failed\n");
    exit();
  }
  print_mem_state("Phase 2: After sbrk(4 * 4096)", &mi_allocated);

  printf(1, "Allocation Verification:\n");
  printf(1, "  User pages delta : +%d pages (Expected: +4)\n", mi_allocated.user_pages - mi_initial.user_pages);
  printf(1, "  Free pages delta : -%d pages (Expected: -4)\n\n", mi_initial.free_pages - mi_allocated.free_pages);

  // Step 3: Fork a child process and examine memory consumption
  printf(1, "--- Forking child process to observe free_pages change ---\n");
  int pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  }

  if(pid == 0){
    // In child process: copyuvm() cloned all user frames and page tables
    struct meminfo mi_child;
    getmeminfo(&mi_child);
    printf(1, "  [CHILD PID %d] Running:\n", getpid());
    printf(1, "    Free Physical Pages : %d\n", mi_child.free_pages);
    printf(1, "    Process User Pages  : %d\n", mi_child.user_pages);
    printf(1, "    Page Table Pages    : %d\n", mi_child.pt_pages);
    exit();
  }

  // In parent: inspect while child exists / reap child
  getmeminfo(&mi_fork);
  wait(); // Child exits and is reaped

  // Step 4: After child exits and is reaped
  if(getmeminfo(&mi_final) < 0){
    printf(1, "getmeminfo failed\n");
    exit();
  }
  print_mem_state("Phase 3: After Child Exits and is Reaped", &mi_final);

  // Step 5: Memory Leak Verification
  printf(1, "--------------------------------------------------------\n");
  printf(1, "Memory Leak Verification:\n");
  printf(1, "  Pre-fork free pages : %d\n", mi_allocated.free_pages);
  printf(1, "  Post-reap free pages: %d\n", mi_final.free_pages);
  
  if(mi_final.free_pages == mi_allocated.free_pages){
    printf(1, "  Delta               : 0 (Exact match!)\n");
    printf(1, "  Status              : [PASSED] ZERO MEMORY LEAK DETECTED!\n");
  } else {
    printf(1, "  Delta               : %d pages\n", mi_allocated.free_pages - mi_final.free_pages);
    printf(1, "  Status              : [FAILED] Memory leak observed!\n");
  }
  printf(1, "========================================================\n");

  exit();
}
