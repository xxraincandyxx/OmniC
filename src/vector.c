// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#include <assert.h>  // For internal sanity checks
#include <omnic/vector.h>
#include <stdlib.h>  // For malloc, realloc, free
#include <string.h>  // For memcpy

#define OC_VECTOR_INITIAL_CAPACITY 8

/* -------------------------------------------------------------------------- */

// --- Struct Definition ---
// This is the actual implementation, hidden from the user.
struct oc_vector {
  char* data;           // Use char* for byte-level pointer arithmetic
  size_t element_size;  // Size of each element in bytes
  size_t size;          // Number of elements currently in the vector
  size_t capacity;      // Number of elements the vector can hold
};

/* -------------------------------------------------------------------------- */

oc_vector_t* oc_vector_create(size_t element_size) {
  if (element_size == 0) {
    return NULL;
  }

  oc_vector_t* vec = malloc(sizeof(oc_vector_t));
  if (!vec) {
    return NULL;
  }

  vec->element_size = element_size;
  vec->size = 0;
  vec->capacity = OC_VECTOR_INITIAL_CAPACITY;
  vec->data = malloc(vec->capacity * vec->element_size);
  if (!vec->data) {
    free(vec);  // Clean up partially allocated struct
    return NULL;
  }

  return vec;
}

void oc_vector_destroy(oc_vector_t* vec) {
  if (vec) {
    free(vec->data);
    free(vec);
  }
}

// Internal helper function to handle resizing
static oc_error_code_t oc_vector_resize(oc_vector_t* vec, size_t new_capacity) {
  if (new_capacity <= vec->capacity) {
    // No need to resize down (for this simple implementation)
    return OC_SUCCESS;
  }

  char* new_data = realloc(vec->data, new_capacity * vec->element_size);
  if (!new_data) {
    return OC_ERROR_ALLOC;
  }

  vec->data = new_data;
  vec->capacity = new_capacity;
  return OC_SUCCESS;
}

oc_error_code_t oc_vector_push_back(oc_vector_t* vec, const void* element) {
  if (!vec || !element) {
    return OC_ERROR_INVALID_ARG;
  }

  // Resize if necessary
  if (vec->size >= vec->capacity) {
    size_t new_capacity =
        vec->capacity == 0 ? OC_VECTOR_INITIAL_CAPACITY : vec->capacity << 1;
    oc_error_code_t err = oc_vector_resize(vec, new_capacity);
    if (err != OC_SUCCESS) {
      return err;
    }
  }

  // Calculate destination address and copy the element
  char* dest = vec->data + (vec->size * vec->element_size);
  memcpy(dest, element, vec->element_size);
  vec->size++;

  return OC_SUCCESS;
}

const void* oc_vector_get(const oc_vector_t* vec, size_t index) {
  if (!vec || index >= vec->size) {
    return NULL;
  }
  return vec->data + (index * vec->element_size);
}

size_t oc_vector_size(const oc_vector_t* vec) { return vec ? vec->size : 0; }

size_t oc_vector_capacity(const oc_vector_t* vec) {
  return vec ? vec->capacity : 0;
}
