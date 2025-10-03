// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <omnic/dynarray.h>
#include <omnic/macros.h>

// --- Test Framework Setup ---

// Global counter for failed tests
static int g_test_failures = 0;

// --- Test Functions ---

void test_initialization_and_free() {
  printf("--- Testing Initialization and Free ---\n");
  int* da = NULL;

  ASSERT_EQ(dalen(da), (size_t)0, "%zu", "Initial length should be 0");
  ASSERT_EQ(dacap(da), (size_t)0, "%zu", "Initial capacity should be 0");
  ASSERT(daempty(da), "Initial array should be empty");

  // Freeing a NULL array should be a safe no-op
  dafree(da);
  ASSERT(da == NULL, "Array should be NULL after freeing a NULL array");

  dapush(da, 100);
  dafree(da);
  ASSERT(da == NULL, "Array pointer should be set to NULL after free");
}

void test_push_pop_last() {
  printf("--- Testing Push, Pop, and Last ---\n");
  int* da = NULL;

  // Push first element, should trigger initial allocation
  dapush(da, 10);
  ASSERT_EQ(dalen(da), (size_t)1, "%zu", "Length should be 1 after one push");
  ASSERT_EQ(dacap(da), (size_t)OC_DYNARRAY_INITIAL_CAPACITY, "%zu",
            "Capacity should be initial capacity");
  ASSERT_EQ(da[0], 10, "%d", "First element should be 10");
  ASSERT(!daempty(da), "Array should not be empty");

  // Push more elements
  dapush(da, 20);
  dapush(da, 30);
  ASSERT_EQ(dalen(da), (size_t)3, "%zu", "Length should be 3");
  ASSERT_EQ(da[1], 20, "%d", "Second element should be 20");
  ASSERT_EQ(da[2], 30, "%d", "Third element should be 30");

  // Test `dalast`
  ASSERT_EQ(dalast(da), 30, "%d", "Last element should be 30");

  // Test `dapop`
  dapop(da);
  ASSERT_EQ(dalen(da), (size_t)2, "%zu", "Length should be 2 after pop");
  ASSERT_EQ(dalast(da), 20, "%d", "New last element should be 20");

  // Pop all elements
  dapop(da);
  dapop(da);
  ASSERT_EQ(dalen(da), (size_t)0, "%zu",
            "Length should be 0 after popping all");
  ASSERT(daempty(da), "Array should be empty after popping all");

  // Test resize logic
  for (int i = 0; i < OC_DYNARRAY_INITIAL_CAPACITY + 1; ++i) {
    dapush(da, i);
  }
  ASSERT_EQ(dalen(da), (size_t)OC_DYNARRAY_INITIAL_CAPACITY + 1, "%zu",
            "Length should be correct after resize");
  ASSERT_EQ(dacap(da), (size_t)OC_DYNARRAY_INITIAL_CAPACITY * 2, "%zu",
            "Capacity should double after resize");
  ASSERT_EQ(da[OC_DYNARRAY_INITIAL_CAPACITY], OC_DYNARRAY_INITIAL_CAPACITY,
            "%d", "Value after resize boundary should be correct");

  dafree(da);
}

void test_insert_and_erase() {
  printf("--- Testing Insert and Erase ---\n");
  int* da = NULL;
  dapush(da, 10);
  dapush(da, 20);
  dapush(da, 30);  // da is now [10, 20, 30]

  // Insert at the beginning
  dainsert(da, 0, 5);  // da should be [5, 10, 20, 30]
  ASSERT_EQ(dalen(da), (size_t)4, "%zu",
            "Length should be 4 after insert at start");
  ASSERT_EQ(da[0], 5, "%d", "New first element should be 5");
  ASSERT_EQ(da[1], 10, "%d", "Shifted element should be 10");

  // Insert in the middle
  dainsert(da, 2, 15);  // da should be [5, 10, 15, 20, 30]
  ASSERT_EQ(dalen(da), (size_t)5, "%zu",
            "Length should be 5 after insert in middle");
  ASSERT_EQ(da[2], 15, "%d", "New middle element should be 15");
  ASSERT_EQ(da[3], 20, "%d", "Shifted element should be 20");

  // Insert at the end (should be same as push)
  dainsert(da, 5, 40);  // da should be [5, 10, 15, 20, 30, 40]
  ASSERT_EQ(dalen(da), (size_t)6, "%zu",
            "Length should be 6 after insert at end");
  ASSERT_EQ(dalast(da), 40, "%d", "New last element should be 40");

  // Erase from the middle
  daerase(da, 2);  // erase 15 -> [5, 10, 20, 30, 40]
  ASSERT_EQ(dalen(da), (size_t)5, "%zu",
            "Length should be 5 after erase from middle");
  ASSERT_EQ(da[2], 20, "%d", "Element after erased one should shift left");

  // Erase from the start
  daerase(da, 0);  // erase 5 -> [10, 20, 30, 40]
  ASSERT_EQ(dalen(da), (size_t)4, "%zu",
            "Length should be 4 after erase from start");
  ASSERT_EQ(da[0], 10, "%d", "New first element should be 10");

  dafree(da);
}

void test_find() {
  printf("--- Testing Find ---\n");
  int* da = NULL;
  dapush(da, 10);
  dapush(da, 20);
  dapush(da, 30);
  dapush(da, 20);  // Add a duplicate

  ASSERT_EQ(dafind(da, 30), (size_t)2, "%zu", "Should find 30 at index 2");
  ASSERT_EQ(dafind(da, 20), (size_t)1, "%zu",
            "Should find first occurrence of 20 at index 1");
  ASSERT_EQ(dafind(da, 99), OC_DYNARRAY_INDEX_NOT_FOUND, "%zu",
            "Should not find non-existent value 99");

  // Test on empty array
  dafree(da);
  ASSERT(da == NULL, "Array should be NULL before find-on-empty test");
  ASSERT_EQ(dafind(da, 10), OC_DYNARRAY_INDEX_NOT_FOUND, "%zu",
            "Find on empty array should not find anything");

  dafree(da);
}

// --- Struct-specific tests ---

typedef struct {
  float x;
  float y;
} point_t;

// Helper to compare structs for tests
bool points_are_equal(point_t a, point_t b) { return a.x == b.x && a.y == b.y; }

void test_structs() {
  printf("--- Testing with Structs ---\n");
  point_t* da = NULL;

  dapush(da, ((point_t){1.1f, 2.2f}));
  dapush(da, ((point_t){3.3f, 4.4f}));

  ASSERT_EQ(dalen(da), (size_t)2, "%zu", "Struct array length should be 2");
  ASSERT(points_are_equal(da[0], ((point_t){1.1f, 2.2f})),
         "First struct has correct values");
  ASSERT(points_are_equal(dalast(da), ((point_t){3.3f, 4.4f})),
         "Last struct has correct values");

  // `dafind` won't work correctly with structs using `==`.
  // A user must implement their own find loop.
  size_t found_idx = OC_DYNARRAY_INDEX_NOT_FOUND;
  point_t to_find = {3.3f, 4.4f};
  for (size_t i = 0; i < dalen(da); ++i) {
    if (points_are_equal(da[i], to_find)) {
      found_idx = i;
      break;
    }
  }
  ASSERT_EQ(found_idx, (size_t)1, "%zu",
            "Custom find loop for structs should work");

  dafree(da);
}

// --- Main Test Runner ---

int main(void) {
  printf("--- Running OmniC Dynarray Test Suite ---\n\n");

  test_initialization_and_free();
  test_push_pop_last();
  test_insert_and_erase();
  test_find();
  test_structs();

  printf("\n--- Test Suite Finished ---\n");
  if (g_test_failures == 0) {
    printf("Result: ALL TESTS PASSED\n");
    return EXIT_SUCCESS;
  }

  {
    printf("Result: %d TEST(S) FAILED\n", g_test_failures);
    return EXIT_FAILURE;
  }
}
