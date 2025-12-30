// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#include "omnic/sorting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_SIZES 4
const int TEST_SIZES[NUM_SIZES] = {500, 10000, 50000, 100000};

typedef void (*sort_func_t)(oc_sort_list_t*);

typedef struct {
  const char* name;
  sort_func_t func;
} algorithm_t;

algorithm_t ALGORITHMS[] = {{"Selection Sort", oc_sort_selection},
                            {"Insertion Sort", oc_sort_insertion},
                            {"Bubble Sort", oc_sort_bubble},
                            {"Quick Sort", oc_sort_quick},
                            {"Merge Sort", oc_sort_merge},
                            {"Heap Sort", oc_sort_heap},
                            {NULL, NULL}};

// Helper to generate random data
void generate_random_data(oc_sort_list_t* list, int n) {
  list->n = n;
  for (int i = 1; i <= n; ++i) {
    list->d[i].key = rand();
    list->d[i].data = i;  // Dummy data
  }
}

// Helper to copy list
void copy_list(oc_sort_list_t* dest, const oc_sort_list_t* src) {
  dest->n = src->n;
  // Only copy valid range to save time?
  // But memcpy is fast. Let's copy strictly 1..n
  // Or just memcpy the whole array if n is close to max.
  // Ideally:
  for (int i = 1; i <= src->n; ++i) {
    dest->d[i] = src->d[i];
  }
}

// Check if sorted
int is_sorted(const oc_sort_list_t* list) {
  for (int i = 1; i < list->n; ++i) {
    if (list->d[i].key > list->d[i + 1].key)
      return 0;
  }
  return 1;
}

int main(void) {
  srand((unsigned int)time(NULL));

  // Allocate lists on heap to avoid stack overflow
  oc_sort_list_t* original = (oc_sort_list_t*)malloc(sizeof(oc_sort_list_t));
  oc_sort_list_t* working = (oc_sort_list_t*)malloc(sizeof(oc_sort_list_t));

  if (!original || !working) {
    fprintf(stderr, "Memory allocation failed\n");
    return 1;
  }

  // Print table header
  printf("+---------------------+------------+-----------+\n");
  printf("| %-19s | %-10s | %-9s |\n", "Algorithm", "Data Size", "Time (ms)");
  printf("+---------------------+------------+-----------+\n");

  for (int s = 0; s < NUM_SIZES; ++s) {
    int n = TEST_SIZES[s];
    generate_random_data(original, n);

    for (int a = 0; ALGORITHMS[a].name != NULL; ++a) {
      // Copy data
      copy_list(working, original);

      // Measure time
      clock_t start = clock();
      ALGORITHMS[a].func(working);
      clock_t end = clock();

      double elapsed_ms = (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;

      // Verify (optional, but good for debugging)
      if (!is_sorted(working)) {
        fprintf(stderr, "Error: %s failed to sort for N=%d\n",
                ALGORITHMS[a].name, n);
      }

      printf("| %-19s | %10d | %9.2f |\n", ALGORITHMS[a].name, n, elapsed_ms);

      // Flush to ensure output is seen immediately
      fflush(stdout);
    }
    // Separator between different sizes
    printf("+---------------------+------------+-----------+\n");
  }

  free(original);
  free(working);

  return 0;
}
