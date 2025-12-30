// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#include "omnic/sorting.h"

#include <stdio.h>
#include <stdlib.h>

// Helper swap function
static inline void swap(oc_sort_entry_t* a, oc_sort_entry_t* b) {
  oc_sort_entry_t temp = *a;
  *a = *b;
  *b = temp;
}

// --- Simple Selection Sort ---
void oc_sort_selection(oc_sort_list_t* list) {
  for (int i = 1; i < list->n; ++i) {
    int min_idx = i;
    for (int j = i + 1; j <= list->n; ++j) {
      if (list->d[j].key < list->d[min_idx].key) {
        min_idx = j;
      }
    }
    if (min_idx != i) {
      swap(&list->d[i], &list->d[min_idx]);
    }
  }
}

// --- Direct Insertion Sort ---
void oc_sort_insertion(oc_sort_list_t* list) {
  for (int i = 2; i <= list->n; ++i) {
    if (list->d[i].key < list->d[i - 1].key) {
      list->d[0] = list->d[i];  // Use d[0] as sentinel
      int j;
      for (j = i - 1; list->d[0].key < list->d[j].key; --j) {
        list->d[j + 1] = list->d[j];
      }
      list->d[j + 1] = list->d[0];
    }
  }
}

// --- Bubble Sort ---
void oc_sort_bubble(oc_sort_list_t* list) {
  for (int i = 1; i < list->n; ++i) {
    int swapped = 0;
    for (int j = 1; j <= list->n - i; ++j) {
      if (list->d[j].key > list->d[j + 1].key) {
        swap(&list->d[j], &list->d[j + 1]);
        swapped = 1;
      }
    }
    if (!swapped)
      break;  // Optimization
  }
}

// --- Quick Sort ---
static int partition(oc_sort_list_t* list, int low, int high) {
  list->d[0] = list->d[low];  // Use d[0] as pivot backup
  oc_key_type_t pivot_key = list->d[low].key;
  while (low < high) {
    while (low < high && list->d[high].key >= pivot_key)
      --high;
    list->d[low] = list->d[high];
    while (low < high && list->d[low].key <= pivot_key)
      ++low;
    list->d[high] = list->d[low];
  }
  list->d[low] = list->d[0];
  return low;
}

static void qsort_recursive(oc_sort_list_t* list, int low, int high) {
  if (low < high) {
    int pivot_loc = partition(list, low, high);
    qsort_recursive(list, low, pivot_loc - 1);
    qsort_recursive(list, pivot_loc + 1, high);
  }
}

void oc_sort_quick(oc_sort_list_t* list) { qsort_recursive(list, 1, list->n); }

// --- Two-way Merge Sort ---
static void merge(oc_sort_entry_t* d, oc_sort_entry_t* temp, int low, int mid,
                  int high) {
  int i = low, j = mid + 1, k = low;
  while (i <= mid && j <= high) {
    if (d[i].key <= d[j].key) {
      temp[k++] = d[i++];
    } else {
      temp[k++] = d[j++];
    }
  }
  while (i <= mid)
    temp[k++] = d[i++];
  while (j <= high)
    temp[k++] = d[j++];
  for (i = low; i <= high; ++i)
    d[i] = temp[i];
}

static void msort_recursive(oc_sort_entry_t* d, oc_sort_entry_t* temp, int low,
                            int high) {
  if (low < high) {
    int mid = (low + high) / 2;
    msort_recursive(d, temp, low, mid);
    msort_recursive(d, temp, mid + 1, high);
    merge(d, temp, low, mid, high);
  }
}

void oc_sort_merge(oc_sort_list_t* list) {
  // Allocate temp array on heap to avoid stack overflow for large N
  oc_sort_entry_t* temp =
      (oc_sort_entry_t*)malloc((list->n + 1) * sizeof(oc_sort_entry_t));
  if (!temp)
    return;  // Allocation failed

  msort_recursive(list->d, temp, 1, list->n);
  free(temp);
}

// --- Heap Sort ---
static void heap_adjust(oc_sort_list_t* list, int s, int m) {
  oc_sort_entry_t rc = list->d[s];
  for (int j = 2 * s; j <= m; j *= 2) {
    if (j < m && list->d[j].key < list->d[j + 1].key)
      ++j;
    if (rc.key >= list->d[j].key)
      break;
    list->d[s] = list->d[j];
    s = j;
  }
  list->d[s] = rc;
}

void oc_sort_heap(oc_sort_list_t* list) {
  // Build heap
  for (int i = list->n / 2; i > 0; --i) {
    heap_adjust(list, i, list->n);
  }
  // Sort
  for (int i = list->n; i > 1; --i) {
    swap(&list->d[1], &list->d[i]);
    heap_adjust(list, 1, i - 1);
  }
}
