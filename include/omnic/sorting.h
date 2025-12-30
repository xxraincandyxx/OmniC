// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#ifndef OMNIC_SORTING_H
#define OMNIC_SORTING_H

#include <stddef.h>  // For size_t

/* -------------------------------------------------------------------------- */

/// @file sorting.h
/// @brief Internal sorting algorithms for the OmniC library.
///
/// Implements various sorting algorithms on a sequential list structure.
/// Based on standard data structure textbook algorithms (Programs 10.1~10.7).

// Maximum size for the static array.
// User requested up to 100,000 elements for benchmarking.
#define OC_SORT_MAX_SIZE 100005

typedef int oc_key_type_t;   ///< The type of the sorting key (comparable).
typedef int oc_data_type_t;  ///< The type of the data payload.

/// @brief Data element structure.
typedef struct {
  oc_key_type_t key;    ///< Sorting key.
  oc_data_type_t data;  ///< Other data.
} oc_sort_entry_t;

/// @brief Sequential list structure for sorting.
///
/// Note: Typically, these algorithms might use index 0 as a sentinel.
/// We will assume the data is stored in d[1]...d[n].
typedef struct {
  int n;                                ///< Number of elements to sort.
  oc_sort_entry_t d[OC_SORT_MAX_SIZE];  ///< Static array of elements.
} oc_sort_list_t;

/* -------------------------------------------------------------------------- */

// --- Sorting Algorithms ---

/// @brief Simple Selection Sort.
/// @param list Pointer to the list to sort.
void oc_sort_selection(oc_sort_list_t* list);

/// @brief Direct Insertion Sort.
/// @param list Pointer to the list to sort.
void oc_sort_insertion(oc_sort_list_t* list);

/// @brief Bubble Sort.
/// @param list Pointer to the list to sort.
void oc_sort_bubble(oc_sort_list_t* list);

/// @brief Quick Sort.
/// @param list Pointer to the list to sort.
void oc_sort_quick(oc_sort_list_t* list);

/// @brief Two-way Merge Sort.
/// @param list Pointer to the list to sort.
void oc_sort_merge(oc_sort_list_t* list);

/// @brief Heap Sort.
/// @param list Pointer to the list to sort.
void oc_sort_heap(oc_sort_list_t* list);

#endif  // OMNIC_SORTING_H
