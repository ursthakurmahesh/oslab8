#ifndef _MEMINFO_H_
#define _MEMINFO_H_

// Question 2: Physical Memory Statistics Structure
struct meminfo {
  int total_pages; // Total physical pages managed by the kernel allocator
  int free_pages;  // Currently available free physical frames
  int user_pages;  // Number of pages in the calling process's user address space (sz / PGSIZE)
  int pt_pages;    // Number of pages used by the process's page directory and page tables
};

#endif
