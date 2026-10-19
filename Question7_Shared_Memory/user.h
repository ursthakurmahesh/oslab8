struct stat;
struct rtcdate;

int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
char* sbrk(int);
int getpid(void);
int shmget(int key, int size);
void* shmat(int shmid);

void printf(int, const char*, ...);
