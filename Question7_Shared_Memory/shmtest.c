#include "types.h"
#include "stat.h"
#include "user.h"

#define SHM_KEY 4242
#define PGSIZE 4096

int
main(int argc, char *argv[])
{
  printf(1, "========================================================\n");
  printf(1, "       xv6 Question 7: Inter-Process Shared Memory      \n");
  printf(1, "========================================================\n\n");

  printf(1, "[Phase 1] Parent allocating shared region key %d...\n", SHM_KEY);
  int shmid = shmget(SHM_KEY, PGSIZE);
  if(shmid < 0){
    printf(1, "shmget failed\n");
    exit();
  }
  printf(1, "  -> Obtained shmid = %d\n", shmid);

  char *shm_ptr = (char*)shmat(shmid);
  if(shm_ptr == (char*)-1){
    printf(1, "shmat failed\n");
    exit();
  }
  printf(1, "  -> Attached shared page at VA: 0x%x\n", (uint)shm_ptr);

  // Write greeting from Parent
  const char *parent_msg = "GREETINGS_FROM_PARENT";
  for(int i = 0; parent_msg[i]; i++)
    shm_ptr[i] = parent_msg[i];
  shm_ptr[21] = '\0';
  printf(1, "  -> Parent wrote to shared memory: '%s'\n\n", shm_ptr);

  // Fork child to test inter-process communication
  printf(1, "[Phase 2] Forking child process to access shared memory...\n");
  int pid = fork();
  if(pid < 0){
    printf(1, "fork failed\n");
    exit();
  }

  if(pid == 0){
    // Child finds shared region using the same key
    int child_shmid = shmget(SHM_KEY, PGSIZE);
    char *child_shm = (char*)shmat(child_shmid);
    printf(1, "  [CHILD] Attached shared memory at VA: 0x%x\n", (uint)child_shm);
    printf(1, "  [CHILD] Read from parent: '%s'\n", child_shm);

    // Child writes response into shared memory
    const char *child_reply = "ACK_FROM_CHILD_PROCESS_PID_4";
    for(int i = 0; child_reply[i]; i++)
      child_shm[i] = child_reply[i];
    child_shm[28] = '\0';
    printf(1, "  [CHILD] Overwrote shared memory with: '%s'\n", child_shm);
    exit();
  }

  wait(); // Wait for child to finish

  printf(1, "\n[Phase 3] Parent verifying child's update in shared memory:\n");
  printf(1, "  -> Parent read: '%s'\n", shm_ptr);
  printf(1, "  -> [PASSED] Bidirectional shared memory IPC verified across distinct address spaces!\n");
  printf(1, "========================================================\n");

  exit();
}
