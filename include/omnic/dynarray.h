// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#ifndef OMNIC_DYNARRAY_H_
#define OMNIC_DYNARRAY_H_

#include <assert.h>   // For internal sanity checks
#include <stdbool.h>  // For boolean values
#include <stddef.h>   // For size_t
#include <stdio.h>    // For fprintf in oc_da_dump
#include <stdlib.h>   // For realloc, free
#include <string.h>   // For memmove

/* -------------------------------------------------------------------------- */

/// @file dynarray.h
/// @brief A generic, type-safe, stb-style dynamic array (dynarray) implementation.
///
/// This implementation uses macros and the "fat pointer" trick to provide a
/// type-safe and convenient API that feels like a language extension.
///
/// USAGE:
/// int* my_da = NULL;
/// oc_da_push(my_da, 10);
/// oc_da_push(my_da, 20);
/// for (size_t i = 0; i < oc_da_len(my_da); ++i) {
///   printf("%d\n", my_da[i]);
/// }
/// oc_da_free(my_da);

/* -------------------------------------------------------------------------- */

// --- Internal Implementation Details ---
// The user should not interact with these directly.

#define OC_DYNARRAY_INITIAL_CAPACITY 8
#define OC_DYNARRAY_INDEX_NOT_FOUND ((size_t)-1)  // Maximum value of size_t

// clang-format off
#ifndef OMNIC_DISABLE_CONDENSED
#define dafree    oc_da_free
// > Capacity
#define dalen     oc_da_len
#define dacap     oc_da_cap
#define daempty   oc_da_empty
// > Modifiers
#define dapush    oc_da_push
#define dapop     oc_da_pop
#define dainsert  oc_da_insert
#define daemplace oc_da_emplace // Alias for insert
#define daerase   oc_da_erase
// > Element Access
#define daat      oc_da_at      // TODO
#define dalast    oc_da_last
#define dafind    oc_da_find
#define dadump    oc_da_dump
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
  // ... (function body remains unchanged)
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

/// @brief Gets the number of elements in the dynarray.
/// @param da The dynarray. Can be NULL (returns 0).
/// @return The number of elements.
#define oc_da_len(da) ((da) ? _oc_da_header(da)->length : 0)

/// @brief Gets the current allocated capacity of the dynarray.
/// @param da The dynarray. Can be NULL (returns 0).
/// @return The capacity.
#define oc_da_cap(da) ((da) ? _oc_da_header(da)->capacity : 0)

/// @brief Checks the emptiness of provided dynarray
/// @param da The dynarray. Can be NULL (returns true).
/// @return Returns a boolean value which determines
/// the emptiness of provided dynarray.
#define oc_da_empty(da) (oc_da_len(da) == 0)

/* -------------------------------------------------------------------------- */

/// @brief Frees all memory used by the dynarray.
/// @param da The dynarray. Can be NULL.
#define oc_da_free(da)         \
  do {                         \
    if (da) {                  \
      free(_oc_da_header(da)); \
      (da) = NULL;             \
    }                          \
  } while ((void)0, 0)

/// @brief Appends an element to the end of the dynarray.
/// @param da The dynarray. **IMPORTANT:** This must be a variable that can be
/// reassigned, as the dynarray's memory block may be moved.
/// @param val The value to push (not a pointer to it).
#define oc_da_push(da, val)                                                  \
  do {                                                                       \
    size_t cap = oc_da_cap(da);                                              \
    size_t len = oc_da_len(da);                                              \
    if (cap <= len) {                                                        \
      size_t new_cap = (cap == 0) ? OC_DYNARRAY_INITIAL_CAPACITY : cap << 1; \
      int err = _oc_da_grow((void**)&(da), new_cap, sizeof(*(da)));          \
      assert(err == 0 && "[OmniC][Dynarray] Failed to grow dynarray");       \
    }                                                                        \
    (da)[len] = (val);                                                       \
    _oc_da_header(da)->length++;                                             \
  } while ((void)0, 0)

/// @brief Removes and returns the last element of the dynarray.
/// Does not shrink the capacity. Asserts if the dynarray is empty.
/// @param da The dynarray.
#define oc_da_pop(da)                                                         \
  do {                                                                        \
    assert(oc_da_len(da) > 0 && "[OmniC][Dynarray] Pop from empty dynarray"); \
    _oc_da_header(da)->length--;                                              \
  } while ((void)0, 0)

/* -------------------------------------------------------------------------- */

/// @brief Returns a reference to the last element in the dynarray.
/// Asserts if the dynarray is empty.
/// @param da The dynarray.
/// @return A reference to the last element.
#define oc_da_last(da) (assert(oc_da_len(da) > 0), (da)[oc_da_len(da) - 1])

/// @brief Inserts an element at a specific index, shifting subsequent elements.
/// @param da The dynarray. Must be a re-assignable variable.
/// @param index The index at which to insert the element. Asserts if out of
/// bounds (must be <= length).
/// @param val The value to insert.
#define oc_da_insert(da, index, val)                                          \
  do {                                                                        \
    size_t len = oc_da_len(da);                                               \
    assert((index) <= len && "[OmniC][Dynarray] Insert index out of bounds"); \
    oc_da_push((da), (val)); /* Grow if needed and temporarily place val */   \
    if ((index) < len) {                                                      \
      memmove(&(da)[(index) + 1], &(da)[(index)],                             \
              (len - (index)) * sizeof(*(da)));                               \
      (da)[(index)] = (val);                                                  \
    }                                                                         \
  } while ((void)0, 0)

/// @brief Alias for oc_da_insert, for users familiar with C++ STL 'emplace'.
#define oc_da_emplace(da, index, val) oc_da_insert(da, index, val)

/// @brief Erases an element at a specific index, shifting subsequent elements.
/// @param da The dynarray.
/// @param index The index of the element to erase. Asserts if out of bounds.
#define oc_da_erase(da, index)                                              \
  do {                                                                      \
    size_t len = oc_da_len(da);                                             \
    assert((index) < len && "[OmniC][Dynarray] Erase index out of bounds"); \
    if ((index) < len - 1) {                                                \
      memmove(&(da)[(index)], &(da)[(index) + 1],                           \
              (len - 1 - (index)) * sizeof(*(da)));                         \
    }                                                                       \
    _oc_da_header(da)->length--;                                            \
  } while ((void)0, 0)

/// @brief Finds the first occurrence of a value in the dynarray.
/// This macro uses the `==` operator for comparison. For complex types
/// (like structs), you may need to write a custom loop.
/// @param da The dynarray.
/// @param val The value to find.
/// @return The index of the first matching element, or
/// OC_DYNARRAY_INDEX_NOT_FOUND if not found.
#define oc_da_find(da, val)                                 \
  ({                                                        \
    size_t _oc_da_find_index = OC_DYNARRAY_INDEX_NOT_FOUND; \
    for (size_t i = 0; i < oc_da_len(da); ++i) {            \
      if ((da)[i] == (val)) {                               \
        _oc_da_find_index = i;                              \
        break;                                              \
      }                                                     \
    }                                                       \
    _oc_da_find_index;                                      \
  })

/// @brief Prints the contents of the dynarray to a stream (C-style
/// `operator<<`).
/// @param da The dynarray.
/// @param fmt A printf-style format specifier for one element (e.g., "%d",
/// "%.2f").
/// @param stream The output stream (e.g., stdout, stderr).
#define oc_da_dump(da, fmt, stream)      \
  do {                                   \
    fprintf((stream), "[");              \
    size_t len = oc_da_len(da);          \
    for (size_t i = 0; i < len; ++i) {   \
      fprintf((stream), (fmt), (da)[i]); \
      if (i < len - 1) {                 \
        fprintf((stream), ", ");         \
      }                                  \
    }                                    \
    fprintf((stream), "]\n");            \
  } while ((void)0, 0)

#endif  // OMNIC_DYNARRAY_H_
