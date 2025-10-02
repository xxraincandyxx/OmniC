// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#include <omnic/dynarray.h>
#include <stdio.h>

typedef struct {
  float x;
  float y;
} point_t;

int main(void) {
  printf("--- OmniC Macro-based Vector Example ---\n\n");

  // --- Example 1: Vector of Integers ---
  printf("--- Integer Vector ---\n");
  int* int_vec = NULL;  // Vectors start as NULL pointers

  printf("Initial state: length=%zu, capacity=%zu\n", dalen(int_vec),
         dacap(int_vec));

  printf("Empty? : %d\n", daempty(int_vec));

  printf("Pushing values 10, 20, 30...\n");
  dapush(int_vec, 10);
  dapush(int_vec, 20);
  dapush(int_vec, 30);

  printf("Empty? : %d\n", daempty(int_vec));

  printf("State after push: length=%zu, capacity=%zu\n", dalen(int_vec),
         dacap(int_vec));

  printf("Contents (using standard array access):\n");
  for (size_t i = 0; i < dalen(int_vec); ++i) {
    printf("  int_vec[%zu] = %d\n", i, int_vec[i]);
  }

  printf("Last element is: %d\n", dalast(int_vec));
  dapop(int_vec);
  printf("After pop, length is %zu, last element is: %d\n", dalen(int_vec),
         dalast(int_vec));

  dafree(int_vec);
  printf("Vector freed. Pointer is now %p\n\n", (void*)int_vec);

  // --- Example 2: Vector of Structs ---
  printf("--- Struct Vector ---\n");
  point_t* point_vec = NULL;

  printf("Empty? : %d\n", daempty(int_vec));

  // Note: for structs, we can push them directly
  dapush(point_vec, ((point_t){1.1f, 2.2f}));
  dapush(point_vec, ((point_t){3.3f, 4.4f}));

  printf("Empty? : %d\n", daempty(int_vec));

  printf("Point vector contents:\n");
  for (size_t i = 0; i < dalen(point_vec); ++i) {
    printf("  point_vec[%zu] = { x: %.1f, y: %.1f }\n", i, point_vec[i].x,
           point_vec[i].y);
  }
  dafree(point_vec);

  return 0;
}
