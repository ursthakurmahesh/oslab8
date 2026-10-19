struct stat;
struct rtcdate;

// system calls
int fork(void);
int exit(void) __attribute__((noreturn));
int wait(void);
char* sbrk(int);
int getpid(void);

// Assignment 8: Question 3
int vmprint(void);

// ulib.c
void printf(int, const char*, ...);
