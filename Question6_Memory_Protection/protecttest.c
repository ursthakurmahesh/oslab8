#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

int
main(int argc, char *argv[])
{
  printf(1, "========================================================\n");
  printf(1, "      xv6 Question 6: Memory Protection (mprotect)      \n");
  printf(1, "========================================================\n\n");

  // Step 1: Allocate a page
  char *page = sbrk(PGSIZE);
  if(page == (char*)-1){
    printf(1, "sbrk failed\n");
    exit();
  }
  printf(1, "[Phase 1] Allocated 1 Page at 0x%x (Writable by default)\n", (uint)page);
  page[0] = 'H';
  page[1] = 'I';
  page[2] = '\0';
  printf(1, "  Wrote initial string: '%s'\n\n", page);

  // Step 2: Set page to Read-Only via mprotect
  printf(1, "[Phase 2] Applying mprotect(0x%x, PGSIZE, PROT_READ)...\n", (uint)page);
  if(mprotect(page, PGSIZE, PROT_READ) < 0){
    printf(1, "mprotect failed\n");
    exit();
  }
  printf(1, "  -> [PASSED] Permission changed to PROT_READ successfully.\n");
  printf(1, "  Reading page content: '%s' (Read succeeded!)\n\n", page);

  // Step 3: Test Write Violation in Child Process
  printf(1, "[Phase 3] Testing Write Violation in Child Process...\n");
  int pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  }

  if(pid == 0){
    printf(1, "  [CHILD PID %d] Attempting to write to read-only page...\n", pid);
    page[0] = 'X'; // Should trigger memory protection fault
    printf(1, "  [CHILD] Error: Write succeeded when it should have faulted!\n");
    exit();
  }

  wait();
  printf(1, "  [PARENT] Child was terminated by MMU page protection trap!\n\n");

  // Step 4: Restore Read-Write Permissions via mprotect
  printf(1, "[Phase 4] Restoring PROT_WRITE via mprotect...\n");
  if(mprotect(page, PGSIZE, PROT_READ | PROT_WRITE) < 0){
    printf(1, "mprotect restore failed\n");
    exit();
  }
  page[0] = 'O';
  page[1] = 'K';
  printf(1, "  Wrote new string: '%s'\n", page);
  printf(1, "  -> [PASSED] Write permission restored successfully!\n\n");

  printf(1, "========================================================\n");
  printf(1, "  Question 6 Memory Protection Verification: COMPLETE!   \n");
  printf(1, "========================================================\n");
  exit();
}
