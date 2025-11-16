// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Includes the generic binary tree API
#include <omnic/binarytree.h>

/* -------------------------------------------------------------------------- */
// --- Test Framework Setup (Copied from Dynarray Test Style) ---
/* -------------------------------------------------------------------------- */

// Global counter for failed tests
static int g_test_failures = 0;

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define ASSERT(condition, message)                                          \
  do {                                                                      \
    if (!(condition)) {                                                     \
      g_test_failures++;                                                    \
      fprintf(stderr, ANSI_COLOR_RED "[FAIL] %s:%d: %s\n" ANSI_COLOR_RESET, \
              __FILE__, __LINE__, message);                                 \
    } else {                                                                \
      printf(ANSI_COLOR_GREEN "[PASS] %s\n" ANSI_COLOR_RESET, message);     \
    }                                                                       \
  } while (0)

#define ASSERT_EQ(actual, expected, fmt, message)                              \
  do {                                                                         \
    if (!((actual) == (expected))) {                                           \
      g_test_failures++;                                                       \
      fprintf(stderr,                                                          \
              ANSI_COLOR_RED "[FAIL] %s:%d: %s - Expected: " fmt ", Got: " fmt \
                             "\n" ANSI_COLOR_RESET,                            \
              __FILE__, __LINE__, message, (expected), (actual));              \
    } else {                                                                   \
      printf(ANSI_COLOR_GREEN "[PASS] %s\n" ANSI_COLOR_RESET, message);        \
    }                                                                          \
  } while (0)

/* -------------------------------------------------------------------------- */
// --- Helper Functions for Generic Data ---
/* -------------------------------------------------------------------------- */

// Global buffer to store traversal results (for verification)
static int g_traversal_buffer[16];
static size_t g_buffer_index = 0;

/// @brief Traversal callback function: Prints an integer and buffers it.
void print_and_buffer_int(const void* data) {
  if (data) {
    printf("%d ", *(const int*)data);
    if (g_buffer_index < 16) {
      g_traversal_buffer[g_buffer_index++] = *(const int*)data;
    }
  }
}

/// @brief Data destructor function: Frees a heap-allocated integer.
void free_int(void* data) {
  if (data) {
    free(data);
  }
}

/// @brief Helper to allocate and set a heap-allocated integer.
int* allocate_int(int value) {
  int* ptr = (int*)malloc(sizeof(int));
  if (ptr) {
    *ptr = value;
  }
  return ptr;
}

/// @brief Builds a standard test tree and returns the root.
///
/// Tree structure:
///      10
///     /  \
///   20    30
///  / \   /
/// 40 50 60
oc_bintree_node_t* build_test_tree() {
  oc_bintree_node_t* n10 = oc_bintree_create_node(allocate_int(10));
  oc_bintree_node_t* n20 = oc_bintree_create_node(allocate_int(20));
  oc_bintree_node_t* n30 = oc_bintree_create_node(allocate_int(30));
  oc_bintree_node_t* n40 = oc_bintree_create_node(allocate_int(40));
  oc_bintree_node_t* n50 = oc_bintree_create_node(allocate_int(50));
  oc_bintree_node_t* n60 = oc_bintree_create_node(allocate_int(60));

  oc_bintree_set_left(n10, n20);
  oc_bintree_set_right(n10, n30);
  oc_bintree_set_left(n20, n40);
  oc_bintree_set_right(n20, n50);
  oc_bintree_set_left(n30, n60);

  return n10;
}

/* -------------------------------------------------------------------------- */
// --- Test Functions ---
/* -------------------------------------------------------------------------- */

void test_creation_and_destruction() {
  printf("--- Testing Creation and Destruction ---\n");

  // Test: Simple tree using heap-allocated data (requires dtor)
  // The data itself must be allocated on the heap if we pass a dtor like
  // free_int.
  int* heap_val = allocate_int(100);
  oc_bintree_node_t* root = oc_bintree_create_node(heap_val);

  ASSERT(root != NULL, "Root node creation successful");
  ASSERT_EQ(*(int*)root->data, 100, "%d", "Node data stored correctly");

  // Add another heap node
  oc_bintree_set_left(root, oc_bintree_create_node(allocate_int(50)));
  ASSERT(root->left != NULL, "Setting left child successful");

  // Destroying the small tree. Both data pointers now point to heap memory.
  oc_bintree_destroy(root, free_int);
  printf("[NOTE] Simple tree destroyed successfully using free_int dtor.\n");

  // Test 2: Test with complex structure and custom destructor
  oc_bintree_node_t* test_root = build_test_tree();
  ASSERT(test_root != NULL, "Complex tree built successfully");
  // This will clean up all 6 nodes and the 6 heap-allocated integers
  oc_bintree_destroy(test_root, free_int);

  printf(
      "Destruction test (memory cleanup) relies on Valgrind/external "
      "checks.\n\n");
}

void test_traversals() {
  printf("--- Testing Traversals ---\n");
  oc_bintree_node_t* root = build_test_tree();

  int expected_preorder[] = {10, 20, 40, 50, 30, 60};
  int expected_inorder[] = {40, 20, 50, 10, 60, 30};
  int expected_postorder[] = {40, 50, 20, 60, 30, 10};
  size_t expected_size = 6;

  // --- Pre-order Test ---
  g_buffer_index = 0;
  printf("Pre-order: ");
  oc_bintree_traverse(root, OC_BINTREE_PREORDER, print_and_buffer_int);
  printf("\n");
  bool pre_match = (g_buffer_index == expected_size) &&
                   (memcmp(g_traversal_buffer, expected_preorder,
                           expected_size * sizeof(int)) == 0);
  ASSERT(pre_match, "Pre-order traversal matches expected sequence");

  // --- In-order Test ---
  g_buffer_index = 0;
  printf("In-order:  ");
  oc_bintree_traverse(root, OC_BINTREE_INORDER, print_and_buffer_int);
  printf("\n");
  bool in_match = (g_buffer_index == expected_size) &&
                  (memcmp(g_traversal_buffer, expected_inorder,
                          expected_size * sizeof(int)) == 0);
  ASSERT(in_match, "In-order traversal matches expected sequence");

  // --- Post-order Test ---
  g_buffer_index = 0;
  printf("Post-order: ");
  oc_bintree_traverse(root, OC_BINTREE_POSTORDER, print_and_buffer_int);
  printf("\n");
  bool post_match = (g_buffer_index == expected_size) &&
                    (memcmp(g_traversal_buffer, expected_postorder,
                            expected_size * sizeof(int)) == 0);
  ASSERT(post_match, "Post-order traversal matches expected sequence");

  oc_bintree_destroy(root, free_int);
  printf("\n");
}

void test_calculations() {
  printf("--- Testing Node, Leaf, and Height Calculations ---\n");
  oc_bintree_node_t* root = build_test_tree();
  oc_bintree_node_t* null_root = NULL;

  // --- Size Test ---
  ASSERT_EQ(oc_bintree_size(root), (size_t)6, "%zu",
            "Total node size calculation");
  ASSERT_EQ(oc_bintree_size(null_root), (size_t)0, "%zu",
            "Node size of a NULL tree is 0");

  // --- Leaves Test ---
  // Leaves are 40, 50, 60 (3 nodes)
  ASSERT_EQ(oc_bintree_leaves(root), (size_t)3, "%zu",
            "Total leaf count calculation");
  ASSERT_EQ(oc_bintree_leaves(null_root), (size_t)0, "%zu",
            "Leaf count of a NULL tree is 0");

  // --- Height Test ---
  // Path: 10 -> 20 -> 40 (Height = 3)
  ASSERT_EQ(oc_bintree_height(root), (size_t)3, "%zu",
            "Height calculation (max depth)");
  ASSERT_EQ(oc_bintree_height(null_root), (size_t)0, "%zu",
            "Height of a NULL tree is 0");

  oc_bintree_destroy(root, free_int);

  // Single node height test
  oc_bintree_node_t* single = oc_bintree_create_node(allocate_int(1));
  ASSERT_EQ(oc_bintree_height(single), (size_t)1, "%zu",
            "Height of a single node tree is 1");
  oc_bintree_destroy(single, free_int);

  printf("\n");
}

void test_mirror() {
  printf("--- Testing Mirror (Switch Left/Right) ---\n");
  oc_bintree_node_t* root = build_test_tree();

  // Initial Pre-order: 10, 20, 40, 50, 30, 60
  // Mirrored Tree structure:
  //       10
  //      /  \
  //     30   20
  //     |    / \
  //     60  50 40
  //
  // Expected Mirrored Pre-order (Root, L, R): 10, 30, 60, 20, 50, 40
  int expected_mirrored_preorder[] = {10, 30, 60, 20, 50, 40};
  size_t expected_size = 6;

  oc_bintree_mirror(root);
  printf("Tree mirrored. New Pre-order: ");

  // --- Verify Traversal After Mirror ---
  g_buffer_index = 0;
  oc_bintree_traverse(root, OC_BINTREE_PREORDER, print_and_buffer_int);
  printf("\n");
  bool mirror_match = (g_buffer_index == expected_size) &&
                      (memcmp(g_traversal_buffer, expected_mirrored_preorder,
                              expected_size * sizeof(int)) == 0);
  ASSERT(mirror_match,
         "Mirrored tree Pre-order traversal matches expected sequence");

  // Test height remains the same
  ASSERT_EQ(oc_bintree_height(root), (size_t)3, "%zu",
            "Height must remain unchanged after mirroring");

  oc_bintree_destroy(root, free_int);
  printf("\n");
}

/* -------------------------------------------------------------------------- */
// --- Main Test Runner ---
/* -------------------------------------------------------------------------- */

int main(void) {
  printf("--- Running OmniC Binary Tree Test Suite ---\n\n");

  test_creation_and_destruction();
  test_traversals();
  test_calculations();
  test_mirror();

  printf("\n--- Test Suite Finished ---\n");
  if (g_test_failures == 0) {
    printf(ANSI_COLOR_GREEN "Result: ALL TESTS PASSED\n" ANSI_COLOR_RESET);
    return EXIT_SUCCESS;
  }

  {
    fprintf(stderr,
            ANSI_COLOR_RED "Result: %d TEST(S) FAILED\n" ANSI_COLOR_RESET,
            g_test_failures);
    return EXIT_FAILURE;
  }
}