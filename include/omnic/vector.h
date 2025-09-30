#ifndef OMNIC_VECTOR_H
#define OMNIC_VECTOR_H

#include <omnic/common.h>

/**
 * @file vector.h
 * @brief A generic, dynamic array (vector) implementation.
 *
 * This API provides a type-safe way to manage a dynamic array of any element type.
 */

// --- Opaque Pointer Declaration ---
// The user knows this type exists, but not what's inside it.
// This is the core of C-style encapsulation.
typedef struct oc_vector oc_vector_t;

/**
 * @brief Creates a new vector.
 *
 * @param element_size The size in bytes of a single element (e.g., sizeof(int)).
 * @return A pointer to the newly created vector, or NULL on allocation failure.
 */
oc_vector_t* oc_vector_create(size_t element_size);

/**
 * @brief Destroys a vector and frees all associated memory.
 *
 * @param vec A pointer to the vector to be destroyed. If NULL, the function does nothing.
 */
void oc_vector_destroy(oc_vector_t* vec);

/**
 * @brief Appends an element to the end of the vector.
 *
 * The vector will automatically resize if its capacity is exceeded.
 *
 * @param vec A pointer to the vector.
 * @param element A pointer to the element to be copied into the vector.
 * @return OC_SUCCESS on success, or an error code on failure.
 */
oc_error_code_t oc_vector_push_back(oc_vector_t* vec, const void* element);

/**
 * @brief Retrieves a pointer to the element at a specific index.
 *
 * @param vec A pointer to the vector.
 * @param index The index of the element to retrieve.
 * @return A constant pointer to the element. Returns NULL if the index is out of bounds.
 *         The returned pointer is only valid until the next vector modification.
 */
const void* oc_vector_get(const oc_vector_t* vec, size_t index);

/**
 * @brief Gets the number of elements currently in the vector.
 *
 * @param vec A pointer to the vector.
 * @return The number of elements.
 */
size_t oc_vector_size(const oc_vector_t* vec);

/**
 * @brief Gets the total number of elements the vector can hold before resizing.
 *
 * @param vec A pointer to the vector.
 * @return The current capacity.
 */
size_t oc_vector_capacity(const oc_vector_t* vec);

#endif  // OMNIC_VECTOR_H
