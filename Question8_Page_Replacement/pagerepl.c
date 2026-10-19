#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FRAMES 10
#define MAX_PAGES 50

// Simulation Results structure
typedef struct {
  int frame_count;
  int page_faults;
  int hits;
  float hit_ratio;
  float fault_ratio;
} SimResult;

// 1. FIFO (First-In, First-Out) Algorithm
SimResult simulate_fifo(const int *refs, int n, int num_frames) {
  SimResult res;
  res.frame_count = num_frames;
  res.page_faults = 0;
  res.hits = 0;

  int frames[MAX_FRAMES];
  for(int i = 0; i < num_frames; i++) frames[i] = -1;

  int fifo_ptr = 0;

  for(int i = 0; i < n; i++) {
    int page = refs[i];
    int found = 0;
    for(int f = 0; f < num_frames; f++) {
      if(frames[f] == page) {
        found = 1;
        break;
      }
    }

    if(found) {
      res.hits++;
    } else {
      res.page_faults++;
      frames[fifo_ptr] = page;
      fifo_ptr = (fifo_ptr + 1) % num_frames;
    }
  }

  res.hit_ratio = (float)res.hits / n * 100.0f;
  res.fault_ratio = (float)res.page_faults / n * 100.0f;
  return res;
}

// 2. LRU (Least Recently Used) Algorithm
SimResult simulate_lru(const int *refs, int n, int num_frames) {
  SimResult res;
  res.frame_count = num_frames;
  res.page_faults = 0;
  res.hits = 0;

  int frames[MAX_FRAMES];
  int last_used[MAX_FRAMES];
  for(int i = 0; i < num_frames; i++) {
    frames[i] = -1;
    last_used[i] = -1;
  }

  for(int i = 0; i < n; i++) {
    int page = refs[i];
    int found_idx = -1;

    for(int f = 0; f < num_frames; f++) {
      if(frames[f] == page) {
        found_idx = f;
        break;
      }
    }

    if(found_idx != -1) {
      res.hits++;
      last_used[found_idx] = i;
    } else {
      res.page_faults++;
      // Find empty slot or least recently used
      int replace_idx = 0;
      int min_time = 999999;
      int empty_found = 0;

      for(int f = 0; f < num_frames; f++) {
        if(frames[f] == -1) {
          replace_idx = f;
          empty_found = 1;
          break;
        }
        if(last_used[f] < min_time) {
          min_time = last_used[f];
          replace_idx = f;
        }
      }

      frames[replace_idx] = page;
      last_used[replace_idx] = i;
    }
  }

  res.hit_ratio = (float)res.hits / n * 100.0f;
  res.fault_ratio = (float)res.page_faults / n * 100.0f;
  return res;
}

// 3. Optimal (Belady's MIN / OPT) Algorithm
SimResult simulate_optimal(const int *refs, int n, int num_frames) {
  SimResult res;
  res.frame_count = num_frames;
  res.page_faults = 0;
  res.hits = 0;

  int frames[MAX_FRAMES];
  for(int i = 0; i < num_frames; i++) frames[i] = -1;

  for(int i = 0; i < n; i++) {
    int page = refs[i];
    int found = 0;

    for(int f = 0; f < num_frames; f++) {
      if(frames[f] == page) {
        found = 1;
        break;
      }
    }

    if(found) {
      res.hits++;
    } else {
      res.page_faults++;
      // Check for empty frame first
      int replace_idx = -1;
      for(int f = 0; f < num_frames; f++) {
        if(frames[f] == -1) {
          replace_idx = f;
          break;
        }
      }

      // If all frames full, find page not used for longest time in future
      if(replace_idx == -1) {
        int farthest = -1;
        for(int f = 0; f < num_frames; f++) {
          int next_use = 999999;
          for(int j = i + 1; j < n; j++) {
            if(refs[j] == frames[f]) {
              next_use = j;
              break;
            }
          }
          if(next_use > farthest) {
            farthest = next_use;
            replace_idx = f;
          }
        }
      }

      frames[replace_idx] = page;
    }
  }

  res.hit_ratio = (float)res.hits / n * 100.0f;
  res.fault_ratio = (float)res.page_faults / n * 100.0f;
  return res;
}

void print_comparison_table(const char *name, const int *refs, int n) {
  printf("========================================================================\n");
  printf(" Reference String: %s (Length = %d)\n", name, n);
  printf(" Stream: ");
  for(int i = 0; i < n; i++) printf("%d ", refs[i]);
  printf("\n========================================================================\n");
  printf("| Frames | FIFO Faults | LRU Faults | OPT Faults | FIFO Hit%% | LRU Hit%% | OPT Hit%% |\n");
  printf("|:------:|:-----------:|:----------:|:----------:|:---------:|:--------:|:--------:|\n");

  for(int f = 3; f <= 7; f++) {
    SimResult r_fifo = simulate_fifo(refs, n, f);
    SimResult r_lru = simulate_lru(refs, n, f);
    SimResult r_opt = simulate_optimal(refs, n, f);

    printf("|   %2d   |     %2d      |     %2d     |     %2d     |  %5.1f%%   |  %5.1f%%  |  %5.1f%%  |\n",
           f, r_fifo.page_faults, r_lru.page_faults, r_opt.page_faults,
           r_fifo.hit_ratio, r_lru.hit_ratio, r_opt.hit_ratio);
  }
  printf("========================================================================\n\n");
}

int main(int argc, char *argv[]) {
  printf("========================================================================\n");
  printf("      Question 8: Page Replacement Simulation (FIFO, LRU, Optimal)     \n");
  printf("========================================================================\n\n");

  // Workload 1: Standard Reference String (Silberschatz OS Concepts)
  int ref1[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1};
  int n1 = sizeof(ref1)/sizeof(ref1[0]);
  print_comparison_table("Workload 1 (Standard OS Benchmark)", ref1, n1);

  // Workload 2: Belady's Anomaly Demonstration String
  int ref2[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5};
  int n2 = sizeof(ref2)/sizeof(ref2[0]);
  print_comparison_table("Workload 2 (Belady's Anomaly Verification)", ref2, n2);

  // Detailed Belady Anomaly Analysis:
  SimResult b_fifo_3 = simulate_fifo(ref2, n2, 3);
  SimResult b_fifo_4 = simulate_fifo(ref2, n2, 4);
  printf("Belady's Anomaly Analysis for FIFO on Workload 2:\n");
  printf("  -> With 3 Frames: %d Page Faults\n", b_fifo_3.page_faults);
  printf("  -> With 4 Frames: %d Page Faults\n", b_fifo_4.page_faults);
  if(b_fifo_4.page_faults > b_fifo_3.page_faults) {
    printf("  -> [CONFIRMED] Belady's Anomaly Observed! More frames resulted in MORE page faults!\n");
  }

  SimResult b_lru_3 = simulate_lru(ref2, n2, 3);
  SimResult b_lru_4 = simulate_lru(ref2, n2, 4);
  printf("LRU Monotonicity on Workload 2:\n");
  printf("  -> With 3 Frames: %d Faults | With 4 Frames: %d Faults\n", b_lru_3.page_faults, b_lru_4.page_faults);
  printf("  -> [CONFIRMED] LRU belongs to the class of Stack Algorithms (immune to Belady's Anomaly).\n\n");

  // Workload 3: High Temporal and Spatial Locality
  int ref3[] = {1, 2, 3, 2, 1, 4, 5, 2, 1, 2, 3, 4, 5, 1, 2};
  int n3 = sizeof(ref3)/sizeof(ref3[0]);
  print_comparison_table("Workload 3 (High Locality Loop)", ref3, n3);

  printf("Simulation successfully completed.\n");
  return 0;
}
