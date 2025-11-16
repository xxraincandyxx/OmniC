// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#ifndef OMNIC_BINARYTREE_H
#define OMNIC_BINARYTREE_H

#include <stdbool.h>  // For boolean values
#include <stddef.h>   // For size_t
#include <stdlib.h>   // For malloc, free

/* -------------------------------------------------------------------------- */

/// @file binarytree.h
/// @brief A generic, macro-based non-intrusive Binary Tree implementation.
///
/// This implementation uses an opaque node structure and generic functions
/// to handle data of any type (via void pointers).
///
/// **USAGE:**
/// // Define a print function for your data type (e.g., int)
/// void print_int(const void* data) { printf("%d ", *(const int*)data); }
///
/// // Create nodes (assuming data pointers point to heap-allocated data)
/// oc_bintree_node_t* root = oc_bintree_create_node(malloc(sizeof(int)));
/// *(int*)root->data = 50;
///
/// // Traverse the tree
/// oc_bintree_traverse(root, OC_BINTREE_INORDER, print_int);
///
/// // Clean up (requires separate function to free node data)
/// oc_bintree_destroy(root, NULL); // Using NULL dtor for simplicity here

/* -------------------------------------------------------------------------- */

// --- Type Definitions ---

/// @brief Opaque pointer to the binary tree node structure.
typedef struct oc_bintree_node {
  void* data;
  struct oc_bintree_node* left;
  struct oc_bintree_node* right;
} oc_bintree_node_t;

/// @brief Function pointer for a data destructor (freeing element data).
/// @param data A pointer to the element's data to be freed.
typedef void (*oc_bintree_data_dtor_t)(void* data);

/// @brief Function pointer for a traversal callback.
/// @param data A pointer to the node's data.
typedef void (*oc_bintree_traverser_t)(const void* data);

/* -------------------------------------------------------------------------- */

// --- Core API Functions ---

/// @brief Allocates and initializes a new binary tree node.
/// @param data A pointer to the data this node will hold. The tree
///             does not manage this data's memory, only the node structure.
/// @return A pointer to the newly created node, or NULL on allocation failure.
oc_bintree_node_t* oc_bintree_create_node(void* data);

/// @brief Destroys the entire tree starting from the given root, freeing all
///        node structures.
/// @param root The root node of the tree or subtree to destroy.
/// @param dtor An optional destructor function to free the data payload of
///             each node. Can be NULL if the node data should not be freed.
void oc_bintree_destroy(oc_bintree_node_t* root, oc_bintree_data_dtor_t dtor);

/// @brief Sets the left child of a parent node.
/// @param parent The parent node.
/// @param child The node to set as the left child (can be NULL).
void oc_bintree_set_left(oc_bintree_node_t* parent, oc_bintree_node_t* child);

/// @brief Sets the right child of a parent node.
/// @param parent The parent node.
/// @param child The node to set as the right child (can be NULL).
void oc_bintree_set_right(oc_bintree_node_t* parent, oc_bintree_node_t* child);

/* -------------------------------------------------------------------------- */

// --- Traversal and Utility Functions (Internal) ---

/// @brief Internal function for pre-order traversal (Root, Left, Right).
void _oc_bintree_traverse_preorder(oc_bintree_node_t* node,
                                   oc_bintree_traverser_t traverser);

/// @brief Internal function for in-order traversal (Left, Root, Right).
void _oc_bintree_traverse_inorder(oc_bintree_node_t* node,
                                  oc_bintree_traverser_t traverser);

/// @brief Internal function for post-order traversal (Left, Right, Root).
void _oc_bintree_traverse_postorder(oc_bintree_node_t* node,
                                    oc_bintree_traverser_t traverser);

/// @brief Internal function to count the total number of nodes in a tree.
/// @return The total count of nodes (size_t).
size_t _oc_bintree_count_nodes(oc_bintree_node_t* node);

/// @brief Internal function to count the number of leaf nodes in a tree.
/// @return The number of leaf nodes (size_t).
size_t _oc_bintree_count_leaves(oc_bintree_node_t* node);

/// @brief Internal function to calculate the height of the tree.
/// @return The height of the tree (0 for a NULL tree, 1 for a single node).
size_t _oc_bintree_get_height(oc_bintree_node_t* node);

/// @brief Swaps the left and right children recursively (mirrors the tree).
/// @param root The root of the tree or subtree to mirror.
void oc_bintree_mirror(oc_bintree_node_t* root);

/* -------------------------------------------------------------------------- */

// --- Public API Macros (Generic Usage) ---

/// @brief Traversal method enumeration for use in oc_bintree_traverse macro.
// clang-format off
#define OC_BINTREE_PREORDER   _oc_bintree_traverse_preorder
#define OC_BINTREE_INORDER    _oc_bintree_traverse_inorder
#define OC_BINTREE_POSTORDER  _oc_bintree_traverse_postorder
// clang-format on

/// @brief Performs a specified traversal on the tree.
/// @param root The root node of the tree.
/// @param TRAVERSE_TYPE One of OC_BINTREE_PREORDER, OC_BINTREE_INORDER, or
///                      OC_BINTREE_POSTORDER.
/// @param traverser The callback function to execute on each node's data.
#define oc_bintree_traverse(root, TRAVERSE_TYPE, traverser) \
  TRAVERSE_TYPE(root, traverser)

/// @brief Calculates the total number of nodes in the tree.
/// @param root The root node.
/// @return The number of nodes.
#define oc_bintree_size(root) _oc_bintree_count_nodes(root)

/// @brief Calculates the total number of leaf nodes in the tree.
/// @param root The root node.
/// @return The number of leaves.
#define oc_bintree_leaves(root) _oc_bintree_count_leaves(root)

/// @brief Calculates the height (max depth) of the tree.
/// @param root The root node.
/// @return The height.
#define oc_bintree_height(root) _oc_bintree_get_height(root)

#endif  // OMNIC_BINARYTREE_H
