// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#ifndef OMNIC_DS_VECTOR_H
#define OMNIC_DS_VECTOR_H

#include <assert.h>  // For internal sanity checks
#include <stddef.h>  // For size_t
#include <stdlib.h>  // For realloc, free

/**
 * @file ds_vector.h
 * @brief A generic, type-safe, stb-style dynamic array (vector) implementation.
 *
 * This implementation uses macros and the "fat pointer" trick to provide a
 * type-safe and convenient API that feels like a language extension.
 *
 * USAGE:
 *   int* my_vec = NULL;
 *   oc_vec_push(my_vec, 10);
 *   oc_vec_push(my_vec, 20);
 *   for (size_t i = 0; i < oc_vec_len(my_vec); ++i) {
 *     printf("%d\n", my_vec[i]);
 *   }
 *   oc_vec_free(my_vec);
 */

// --- Internal Implementation Details ---
// The user should not interact with these directly.

#define OC_VECTOR_INITIAL_CAPACITY 8

// The metadata header stored *before* the user's data pointer.
typedef struct {
  size_t capacity;
  size_t length;
} oc_vec_header_t;

// Macro to get the header from the user's vector pointer.
#define _oc_vec_header(v) \
  ((oc_vec_header_t*)((char*)(v) - sizeof(oc_vec_header_t)))

// Internal function to handle the growth logic.
// Takes a void** so it can modify the user's original pointer variable.
// Returns 0 on success, -1 on allocation failure.
static inline int _oc_vec_grow(void** v, size_t new_cap, size_t elem_size) {
  // If the vector is NULL, we are creating it for the first time.
  size_t new_block_size = sizeof(oc_vec_header_t) + (new_cap * elem_size);
  oc_vec_header_t* new_header = NULL;

  if (*v == NULL) {
    new_header = (oc_vec_header_t*)malloc(new_block_size);
    if (!new_header) {
      return -1;
    }
    new_header->length = 0;
  } else {
    oc_vec_header_t* old_header = _oc_vec_header(*v);
    new_header = (oc_vec_header_t*)realloc(old_header, new_block_size);
    if (!new_header) {
      return -1;
    }
  }

  new_header->capacity = new_cap;
  // Point the user's vector pointer to the data section.
  *v = (void*)((char*)new_header + sizeof(oc_vec_header_t));
  return 0;
}

// --- Public API Macros ---

/**
 * @brief Gets the number of elements in the vector.
 * @param v The vector. Can be NULL (returns 0).
 * @return The number of elements.
 */
#define oc_vec_len(v) ((v) ? _oc_vec_header(v)->length : 0)

/**
 * @brief Gets the current allocated capacity of the vector.
 * @param v The vector. Can be NULL (returns 0).
 * @return The capacity.
 */
#define oc_vec_cap(v) ((v) ? _oc_vec_header(v)->capacity : 0)

/**
 * @brief Frees all memory used by the vector.
 * @param v The vector. Can be NULL.
 */
#define oc_vec_free(v)         \
  do {                         \
    if (v) {                   \
      free(_oc_vec_header(v)); \
      (v) = NULL;              \
    }                          \
  } while (0)

/**
 * @brief Appends an element to the end of the vector.
 * @param v The vector. IMPORTANT: This must be a variable that can be reassigned,
 *          as the vector's memory block may be moved.
 * @param val The value to push (not a pointer to it).
 */
#define oc_vec_push(v, val)                                               \
  do {                                                                    \
    size_t cap = oc_vec_cap(v);                                           \
    size_t len = oc_vec_len(v);                                           \
    if (cap <= len) {                                                     \
      size_t new_cap = (cap == 0) ? OC_VECTOR_INITIAL_CAPACITY : cap * 2; \
      int err = _oc_vec_grow((void**)&(v), new_cap, sizeof(*(v)));        \
      assert(err == 0 && "Failed to grow vector");                        \
    }                                                                     \
    (v)[len] = (val);                                                     \
    _oc_vec_header(v)->length++;                                          \
  } while (0)

/**
 * @brief Removes and returns the last element of the vector.
 *        Does not shrink the capacity. Asserts if the vector is empty.
 * @param v The vector.
 */
#define oc_vec_pop(v)                                     \
  do {                                                    \
    assert(oc_vec_len(v) > 0 && "Pop from empty vector"); \
    _oc_vec_header(v)->length--;                          \
  } while (0)

/**
 * @brief Returns a reference to the last element in the vector.
 *        Asserts if the vector is empty.
 * @param v The vector.
 * @return A reference to the last element.
 */
#define oc_vec_last(v) (assert(oc_vec_len(v) > 0), (v)[oc_vec_len(v) - 1])

#endif  // OMNIC_DS_VECTOR_H
