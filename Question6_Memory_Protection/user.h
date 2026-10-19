struct stat;
struct rtcdate;

#define PROT_NONE  0x0
#define PROT_READ  0x1
#define PROT_WRITE 0x2

int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
char* sbrk(int);
int mprotect(void *addr, int len, int prot);

void printf(int, const char*, ...);
