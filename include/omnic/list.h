// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#ifndef OMNIC_LIST_H_
#define OMNIC_LIST_H_

#include <assert.h>   // For internal sanity checks
#include <stdbool.h>  // For boolean values
#include <stddef.h>   // For size_t, offsetof
#include <stdio.h>    // For dump/print functions
#include <stdlib.h>   // For malloc, free

/* -------------------------------------------------------------------------- */

/**
 * @file list.h
 * @brief A generic, type-safe, macro-based intrusive doubly-linked list.
 *
 * This implementation uses an "intrusive" design. This means you must embed
 * an `oc_list_node_t` member directly into the structure you wish to link.
 * This approach is highly memory-efficient as it avoids separate allocations
 * for list nodes.
 *
 * USAGE:
 *
 * // 1. Define your data structure with an oc_list_node_t member.
 * typedef struct {
 *   int id;
 *   oc_list_node_t link; // The intrusive node
 * } my_item_t;
 *
 * // 2. Declare and initialize a list head.
 * oc_list_t my_list;
 * oc_list_init(&my_list);
 *
 * // 3. Add items to the list.
 * oc_list_emplace_back(&my_list, my_item_t, link, .id = 10);
 * oc_list_emplace_front(&my_list, my_item_t, link, .id = 5);
 *
 * // 4. Iterate and use the list.
 * my_item_t* item = NULL;
 * oc_list_for_each(item, &my_list, my_item_t, link) {
 *   printf("Item ID: %d\n", item->id);
 * }
 *
 * // 5. Clean up.
 * oc_list_destroy(&my_list, my_item_t, link);
 */

/* -------------------------------------------------------------------------- */

// --- Type Definitions ---

typedef struct oc_list_node_t {
  struct oc_list_node_t* next;
  struct oc_list_node_t* prev;
} oc_list_node_t;

typedef struct {
  oc_list_node_t* head;
  oc_list_node_t* tail;
  size_t size;
} oc_list_t;

/* -------------------------------------------------------------------------- */

// --- Internal Implementation Details ---

// A 'container_of' macro, famously used in the Linux kernel.
// Given a pointer to a member, it returns a pointer to the containing struct.
#define _oc_list_entry(ptr, type, member) \
  ((type*)((char*)(ptr) - offsetof(type, member)))

// Internal function to link a new node between two existing nodes.
static inline void _oc_list_link_node(oc_list_node_t* prev,
                                      oc_list_node_t* next,
                                      oc_list_node_t* new_node) {
  new_node->next = next;
  new_node->prev = prev;
  if (prev) {
    prev->next = new_node;
  }
  if (next) {
    next->prev = new_node;
  }
}

// Internal function to remove a node from the list.
static inline void _oc_list_unlink_node(oc_list_node_t* node) {
  if (node->prev) {
    node->prev->next = node->next;
  }
  if (node->next) {
    node->next->prev = node->prev;
  }
}

// Internal fuction to swap two providing nodes form the list.
// TODO: Complete this macro function.
static inline void _oc_list_swap_node(oc_list_node_t* lhs_node,
                                      oc_list_node_t* rhs_node) {}

/* -------------------------------------------------------------------------- */

// --- Public API Macros ---

/**
 * @brief Initializes a list head to a safe, empty state.
 * @param list Pointer to the oc_list_t object.
 */
#define oc_list_init(list) \
  do {                     \
    (list)->head = NULL;   \
    (list)->tail = NULL;   \
    (list)->size = 0;      \
  } while (0)

/**
 * @brief Gets the number of elements in the list.
 * @param list Pointer to the oc_list_t object.
 */
#define oc_list_size(list) ((list)->size)

/**
 * @brief Checks if the list is empty.
 * @param list Pointer to the oc_list_t object.
 */
#define oc_list_empty(list) (oc_list_size(list) == 0)

/* --- Insertion --- */

/**
 * @brief Allocates and inserts a new element at the front of the list.
 * @param list Pointer to the oc_list_t object.
 * @param type The type of your structure (e.g., my_item_t).
 * @param member The name of the oc_list_node_t member in your struct.
 * @param ... Designated initializers for your struct (e.g., .id = 10).
 * @return Pointer to the newly created element, or NULL on allocation failure.
 */
#define oc_list_emplace_front(list, type, member, ...)            \
  ({                                                              \
    type* new_entry = (type*)malloc(sizeof(type));                \
    if (new_entry) {                                              \
      *new_entry = (type){__VA_ARGS__};                           \
      _oc_list_link_node(NULL, (list)->head, &new_entry->member); \
      if (oc_list_empty(list))                                    \
        (list)->tail = &new_entry->member;                        \
      (list)->head = &new_entry->member;                          \
      (list)->size++;                                             \
    }                                                             \
    new_entry;                                                    \
  })

/**
 * @brief Allocates and inserts a new element at the back of the list.
 * @param list Pointer to the oc_list_t object.
 * @param type The type of your structure (e.g., my_item_t).
 * @param member The name of the oc_list_node_t member in your struct.
 * @param ... Designated initializers for your struct (e.g., .id = 10).
 * @return Pointer to the newly created element, or NULL on allocation failure.
 */
#define oc_list_emplace_back(list, type, member, ...)             \
  ({                                                              \
    type* new_entry = (type*)malloc(sizeof(type));                \
    if (new_entry) {                                              \
      *new_entry = (type){__VA_ARGS__};                           \
      _oc_list_link_node((list)->tail, NULL, &new_entry->member); \
      if (oc_list_empty(list))                                    \
        (list)->head = &new_entry->member;                        \
      (list)->tail = &new_entry->member;                          \
      (list)->size++;                                             \
    }                                                             \
    new_entry;                                                    \
  })

/* --- Iteration and Find --- */

/**
 * @brief Provides a standard for-loop to iterate over the list.
 *
 * @param cursor A pointer variable of your struct's type (e.g., my_item_t*).
 *        It will be assigned to each element in the list.
 * @param list Pointer to the oc_list_t object.
 * @param type The type of your structure.
 * @param member The name of the oc_list_node_t member in your struct.
 *
 * Example:
 *   my_item_t* item;
 *   oc_list_for_each(item, &my_list, my_item_t, link) { ... }
 */
#define oc_list_for_each(cursor, list, type, member)                 \
  for (oc_list_node_t* _node = (list)->head;                         \
       _node && ((cursor) = _oc_list_entry(_node, type, member), 1); \
       _node = _node->next)

/**
 * @brief Provides a safe for-loop for iterating when elements might be erased.
 *
 * @param cursor A pointer variable of your struct's type.
 * @param safe_cursor A temporary pointer variable of the same type.
 * @param list Pointer to the oc_list_t object.
 * @param type The type of your structure.
 * @param member The name of the oc_list_node_t member in your struct.
 */
#define oc_list_for_each_safe(cursor, safe_cursor, list, type, member) \
  for (oc_list_node_t* _node = (list)->head, *_next_node = NULL;       \
       _node && (_next_node = _node->next,                             \
                (cursor) = _oc_list_entry(_node, type, member), 1);    \
       _node = _next_node)

/* --- Invert --- */

// Designed for Singly Linked List (SLL),
// while also compatible with Doubly Linked List (DLL).
#define oc_list_invert(list, type, member) \
  do {                                     \
    oc_list_node_t* prev = NULL;           \
    oc_list_node_t* curr = (list)->head;   \
    oc_list_node_t* next = NULL;           \
    (list)->tail = (list)->head;           \
    while (curr) {                         \
      next = curr->next;                   \
      curr->prev = next;                   \
      curr->next = prev;                   \
      prev = curr;                         \
      curr = next;                         \
    }                                      \
    (list)->head = prev;                   \
  } while (0)

/* --- Erase and Destroy --- */

/**
 * @brief Removes an element from the list and frees its memory.
 * @param list Pointer to the oc_list_t object.
 * @param entry A pointer to the element to erase (e.g., my_item_t*).
 * @param member The name of the oc_list_node_t member in the struct.
 */
#define oc_list_erase(list, entry, member)                                \
  do {                                                                    \
    assert((entry) != NULL && "[OmniC][List] Cannot erase a NULL entry"); \
    oc_list_node_t* node = &(entry)->member;                              \
    if (node == (list)->head)                                             \
      (list)->head = node->next;                                          \
    if (node == (list)->tail)                                             \
      (list)->tail = node->prev;                                          \
    _oc_list_unlink_node(node);                                           \
    (list)->size--;                                                       \
    free(entry);                                                          \
  } while (0)

/**
 * @brief Frees all elements in the list and resets the list head.
 * @param list Pointer to the oc_list_t object.
 * @param type The type of your structure.
 * @param member The name of the oc_list_node_t member in your struct.
 */
#define oc_list_destroy(list, type, member)     \
  do {                                          \
    oc_list_node_t* curr = (list)->head;        \
    while (curr) {                              \
      oc_list_node_t* next = curr->next;        \
      free(_oc_list_entry(curr, type, member)); \
      curr = next;                              \
    }                                           \
    oc_list_init(list);                         \
  } while (0)

/* --- Invert --- */
/**
 * @brief Invert all elements in the list, under time complexity O(n).
 * @param list Pointer to the oc_list_t object.
 * @param type The type of your structure.
 * @param member The name of the oc_list_node_t member in your struct.
 */

/* --- Dump/Print --- */

/**
 * @brief A type for a function that knows how to print one list element.
 * @param entry A void pointer to the user's struct instance.
 * @param stream The file stream to print to.
 */
typedef void (*oc_list_item_dumper_t)(const void* entry, FILE* stream);

/**
 * @brief Dumps the contents of the list to a stream for debugging.
 * @param list Pointer to the oc_list_t object.
 * @param type The type of your structure.
 * @param member The name of the oc_list_node_t member in your struct.
 * @param dumper The function that prints a single element.
 * @param stream The output stream (e.g., stdout).
 */
#define oc_list_dump(list, type, member, dumper, stream)                \
  do {                                                                  \
    fprintf((stream), "oc_list_t (size: %zu) {\n", oc_list_size(list)); \
    type* _entry;                                                       \
    oc_list_for_each(_entry, list, type, member) {                      \
      fprintf((stream), "  ");                                          \
      (dumper)(_entry, stream);                                         \
      fprintf((stream), "\n");                                          \
    }                                                                   \
    fprintf((stream), "}\n");                                           \
  } while (0)

#endif  // OMNIC_LIST_H_
