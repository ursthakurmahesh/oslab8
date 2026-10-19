# Question 8: Page Replacement Algorithms (FIFO, LRU, Optimal)

## Overview
Virtual memory systems rely on page replacement policies to decide which page frame to evict when physical memory is full. This question provides an exhaustive comparative evaluation of three canonical algorithms:
1. **FIFO (First-In, First-Out)**: Evicts the oldest page frame loaded into memory.
2. **LRU (Least Recently Used)**: Evicts the frame unreferenced for the longest period.
3. **Optimal (MIN / Belady's Algorithm)**: Evicts the page that will not be accessed for the furthest time in the future (theoretical upper bound).

---

## Comparison Results

### Workload 1: Standard Benchmark (20 References)
`7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2, 1, 2, 0, 1, 7, 0, 1`

| Frames | FIFO Faults | LRU Faults | OPT Faults | FIFO Hit% | LRU Hit% | OPT Hit% |
|:------:|:-----------:|:----------:|:----------:|:---------:|:--------:|:--------:|
| 3 | 15 | 12 | 9 | 25.0% | 40.0% | 55.0% |
| 4 | 10 | 8 | 8 | 50.0% | 60.0% | 60.0% |
| 5 | 8 | 7 | 7 | 60.0% | 65.0% | 65.0% |
| 6 | 7 | 6 | 6 | 65.0% | 70.0% | 70.0% |
| 7 | 6 | 6 | 6 | 70.0% | 70.0% | 70.0% |

---

## Belady's Anomaly Verification
On the reference string `1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5`:
- **FIFO with 3 Frames**: 9 page faults.
- **FIFO with 4 Frames**: **10 page faults** (Anomaly detected: fault count increased despite added physical capacity).
- **LRU**: Monotonically decreases (10 faults with 3 frames $\to$ 8 faults with 4 frames). Proof that Stack Algorithms never experience Belady's Anomaly.
