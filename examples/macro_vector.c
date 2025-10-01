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
    int* int_vec = NULL; // Vectors start as NULL pointers

    printf("Initial state: length=%zu, capacity=%zu\n",
           oc_da_len(int_vec), oc_da_cap(int_vec));

    printf("Pushing values 10, 20, 30...\n");
    oc_da_push(int_vec, 10);
    oc_da_push(int_vec, 20);
    oc_da_push(int_vec, 30);

    printf("State after push: length=%zu, capacity=%zu\n",
           oc_da_len(int_vec), oc_da_cap(int_vec));

    printf("Contents (using standard array access):\n");
    for (size_t i = 0; i < oc_da_len(int_vec); ++i) {
        printf("  int_vec[%zu] = %d\n", i, int_vec[i]);
    }

    printf("Last element is: %d\n", oc_da_last(int_vec));
    oc_da_pop(int_vec);
    printf("After pop, length is %zu, last element is: %d\n",
           oc_da_len(int_vec), oc_da_last(int_vec));

    oc_da_free(int_vec);
    printf("Vector freed. Pointer is now %p\n\n", (void*)int_vec);


    // --- Example 2: Vector of Structs ---
    printf("--- Struct Vector ---\n");
    point_t* point_vec = NULL;

    // Note: for structs, we can push them directly
    oc_da_push(point_vec, ((point_t){1.1f, 2.2f}));
    oc_da_push(point_vec, ((point_t){3.3f, 4.4f}));

    printf("Point vector contents:\n");
    for (size_t i = 0; i < oc_da_len(point_vec); ++i) {
        printf("  point_vec[%zu] = { x: %.1f, y: %.1f }\n",
               i, point_vec[i].x, point_vec[i].y);
    }
    oc_da_free(point_vec);

    return 0;
}
