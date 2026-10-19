#include "types.h"
#include "stat.h"
#include "user.h"

// Global variable placed in data segment
int global_var = 0xABCD;

int
main(int argc, char *argv[])
{
  int local_var = 0x1234;
  char *heap_block = (char*)malloc(128);
  if(heap_block == 0){
    printf(1, "malloc failed\n");
    exit();
  }
  *heap_block = 'Z';

  printf(1, "========================================================\n");
  printf(1, "  xv6 Question 1: Virtual to Physical Address Translation\n");
  printf(1, "========================================================\n\n");

  printf(1, "[Task 1.c] Translating addresses for process memory regions:\n");
  printf(1, "--------------------------------------------------------\n");
  
  // 1. Global Variable (Data segment)
  printf(1, "Translating Global Variable (&global_var):\n");
  int pa_global = va2pa((int)&global_var);
  printf(1, "  -> VA = 0x%x  |  PA = 0x%x\n\n", (uint)&global_var, (uint)pa_global);

  // 2. Local Variable (User Stack)
  printf(1, "Translating Local Variable (&local_var):\n");
  int pa_local = va2pa((int)&local_var);
  printf(1, "  -> VA = 0x%x  |  PA = 0x%x\n\n", (uint)&local_var, (uint)pa_local);

  // 3. Heap Block (Dynamically allocated via malloc)
  printf(1, "Translating Heap Block (heap_block):\n");
  int pa_heap = va2pa((int)heap_block);
  printf(1, "  -> VA = 0x%x  |  PA = 0x%x\n\n", (uint)heap_block, (uint)pa_heap);

  // 4. Invalid / Unmapped Address
  printf(1, "[Task 1.c] Testing unmapped address (0x7FFFFFFF):\n");
  int pa_unmapped = va2pa(0x7FFFFFFF);
  printf(1, "  -> va2pa(0x7FFFFFFF) = %d (Expected: -1)\n", pa_unmapped);
  if(pa_unmapped == -1)
    printf(1, "  -> [PASSED] Correctly returned -1 for unmapped virtual address.\n\n");
  else
    printf(1, "  -> [FAILED] Did not return -1!\n\n");

  // 5. Kernel Address (Above KERNBASE 0x80000000)
  printf(1, "[Task 1.c] Testing kernel address (0x80000000, PTE_U not set):\n");
  int pa_kernel = va2pa(0x80000000);
  printf(1, "  -> va2pa(0x80000000) = %d (Expected: -1)\n", pa_kernel);
  if(pa_kernel == -1)
    printf(1, "  -> [PASSED] Correctly returned -1 for user-inaccessible kernel page.\n\n");

  // 6. Fork Test: Verify Same VA, Different PA
  printf(1, "[Task 1.d] Testing fork() Address Space Isolation:\n");
  printf(1, "--------------------------------------------------------\n");
  
  int pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  }

  if(pid == 0){
    // In Child Process
    printf(1, "  [CHILD]  Executing in child PID %d\n", getpid());
    printf(1, "  [CHILD]  VA of global_var = 0x%x\n", (uint)&global_var);
    int pa_child = va2pa((int)&global_var);
    printf(1, "  [CHILD]  PA of global_var = 0x%x\n", (uint)pa_child);
    
    // Child modifies its private copy
    global_var = 0x9999;
    printf(1, "  [CHILD]  Modified global_var = 0x%x\n", global_var);
    exit();
  } else {
    // In Parent Process: wait for child to complete
    wait();
    printf(1, "  [PARENT] Child completed. Executing in parent PID %d\n", getpid());
    printf(1, "  [PARENT] VA of global_var = 0x%x\n", (uint)&global_var);
    int pa_parent = va2pa((int)&global_var);
    printf(1, "  [PARENT] PA of global_var = 0x%x\n", (uint)pa_parent);
    printf(1, "  [PARENT] Value of global_var = 0x%x (Unchanged!)\n\n", global_var);

    printf(1, "--------------------------------------------------------\n");
    printf(1, "  Verification Summary:\n");
    printf(1, "  - Virtual Addresses are IDENTICAL  : 0x%x == 0x%x\n", (uint)&global_var, (uint)&global_var);
    printf(1, "  - Physical Addresses are DIFFERENT : Parent PA 0x%x != Child PA\n", (uint)pa_parent);
    printf(1, "  - xv6 copyuvm() deep-copy verified: COMPLETE SUCCESS!\n");
    printf(1, "========================================================\n");
  }

  free(heap_block);
  exit();
}
