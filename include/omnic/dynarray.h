// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#ifndef OMNIC_DYNARRAY_H_
#define OMNIC_DYNARRAY_H_

#include <assert.h>   // For internal sanity checks
#include <stdbool.h>  // For boolean values
#include <stddef.h>   // For size_t
#include <stdlib.h>   // For realloc, free
#include <string.h>   // For memcpy in complex implementations
                      // (not used here, but good practice)

/* -------------------------------------------------------------------------- */

/**
 * @file dynarray.h
 * @brief A generic, type-safe, stb-style
 * dynamic array (dynarray) implementation.
 *
 * This implementation uses macros and the "fat pointer" trick to provide a
 * type-safe and convenient API that feels like a language extension.
 *
 * USAGE:
 * int* my_da = NULL;
 * oc_da_push(my_da, 10);
 * oc_da_push(my_da, 20);
 * for (size_t i = 0; i < oc_da_len(my_da); ++i) {
 *   printf("%d\n", my_da[i]);
 * }
 * oc_da_free(my_da);
 */

/* -------------------------------------------------------------------------- */

// --- Internal Implementation Details ---
// The user should not interact with these directly.

#define OC_DYNARRAY_INITIAL_CAPACITY 8

// clang-format off
#ifndef OMNIC_DISABLE_CONDENSED
#define dalen    oc_da_len
#define dacap    oc_da_cap
#define daempty  oc_da_empty
#define dapush   oc_da_push
#define dapop    oc_da_pop
#define dalast   oc_da_last
#define dafree   oc_da_free
#endif
// clang-format on

/* -------------------------------------------------------------------------- */

// The metadata header stored *before* the user's data pointer.
typedef struct {
  size_t capacity;
  size_t length;
} oc_da_header_t;

// Macro to get the header from the user's dynarray pointer.
#define _oc_da_header(da) \
  ((oc_da_header_t*)((char*)(da) - sizeof(oc_da_header_t)))

/* -------------------------------------------------------------------------- */

// Internal function to handle the growth logic.
// Takes a void** so it can modify the user's original pointer variable.
// Returns 0 on success, -1 on allocation failure.
static inline int _oc_da_grow(void** da, size_t new_cap, size_t elem_size) {
  // If the dynarray is NULL, we are creating it for the first time.
  size_t new_block_size = sizeof(oc_da_header_t) + (new_cap * elem_size);
  oc_da_header_t* new_header = NULL;

  if (*da == NULL) {
    new_header = (oc_da_header_t*)malloc(new_block_size);
    if (!new_header) {
      return -1;
    }
    new_header->length = 0;
  } else {
    oc_da_header_t* old_header = _oc_da_header(*da);
    new_header = (oc_da_header_t*)realloc(old_header, new_block_size);
    if (!new_header) {
      return -1;
    }
  }

  new_header->capacity = new_cap;
  // Point the user's dynarray pointer to the data section.
  *da = (void*)((char*)new_header + sizeof(oc_da_header_t));
  return 0;
}

/* -------------------------------------------------------------------------- */

// --- Public API Macros ---

/**
 * @brief Gets the number of elements in the dynarray.
 * @param da The dynarray. Can be NULL (returns 0).
 * @return The number of elements.
 */
#define oc_da_len(da) ((da) ? _oc_da_header(da)->length : 0)

/**
 * @brief Gets the current allocated capacity of the dynarray.
 * @param da The dynarray. Can be NULL (returns 0).
 * @return The capacity.
 */
#define oc_da_cap(da) ((da) ? _oc_da_header(da)->capacity : 0)

/**
 * @brief Checks the emptiness of provided dynarray
 * @param da The dynarray. Can be NULL (returns true).
 * @return Returns a boolean value which determines
 * the emptiness of provided dynarray.
 */
#define oc_da_empty(da) \
  ((da) ? (_oc_da_header(da)->length == 0 ? true : false) : true)

/**
 * @brief Frees all memory used by the dynarray.
 * @param da The dynarray. Can be NULL.
 */
#define oc_da_free(da)         \
  do {                         \
    if (da) {                  \
      free(_oc_da_header(da)); \
      (da) = NULL;             \
    }                          \
  } while (0)

/**
 * @brief Appends an element to the end of the dynarray.
 * @param da The dynarray. IMPORTANT: This must be a variable that can be
 * reassigned, as the dynarray's memory block may be moved.
 * @param val The value to push (not a pointer to it).
 */
#define oc_da_push(da, val)                                                  \
  do {                                                                       \
    size_t cap = oc_da_cap(da);                                              \
    size_t len = oc_da_len(da);                                              \
    if (cap <= len) {                                                        \
      size_t new_cap = (cap == 0) ? OC_DYNARRAY_INITIAL_CAPACITY : cap << 1; \
      int err = _oc_da_grow((void**)&(da), new_cap, sizeof(*(da)));          \
      assert(err == 0 && "[Omnic][Dynarray] Failed to grow dynarray");       \
    }                                                                        \
    (da)[len] = (val);                                                       \
    _oc_da_header(da)->length++;                                             \
  } while (0)

/**
 * @brief Removes and returns the last element of the dynarray.
 * Does not shrink the capacity. Asserts if the dynarray is empty.
 * @param da The dynarray.
 */
#define oc_da_pop(da)                                                         \
  do {                                                                        \
    assert(oc_da_len(da) > 0 && "[Omnic][Dynarray] Pop from empty dynarray"); \
    _oc_da_header(da)->length--;                                              \
  } while (0)

/**
 * @brief Returns a reference to the last element in the dynarray.
 * Asserts if the dynarray is empty.
 * @param da The dynarray.
 * @return A reference to the last element.
 */
#define oc_da_last(da) (assert(oc_da_len(da) > 0), (da)[oc_da_len(da) - 1])

#endif  // OMNIC_DYNARRAY_H_
