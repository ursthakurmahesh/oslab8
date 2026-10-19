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
int pipe(int*);
int write(int, const void*, int);
int read(int, void*, int);
int close(int);
int kill(int);
int getpid(void);
char* sbrk(int);
int getmeminfo(struct meminfo *mi);

void printf(int, const char*, ...);
