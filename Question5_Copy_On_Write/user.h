struct stat;
struct rtcdate;
struct meminfo {
  int total_pages;
  int free_pages;
  int user_pages;
  int pt_pages;
};

int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
char* sbrk(int);
int getpid(void);
int getmeminfo(struct meminfo *mi);

void printf(int, const char*, ...);
