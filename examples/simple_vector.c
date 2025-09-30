#include <omnic/vector.h>
#include <stdio.h>

int main() {
    printf("--- OmniC Vector Example ---\n");

    // Create a vector to hold integers
    oc_vector_t* int_vec = oc_vector_create(sizeof(int));
    if (!int_vec) {
        fprintf(stderr, "Failed to create vector.\n");
        return 1;
    }

    printf("Vector created. Initial size: %zu, capacity: %zu\n",
           oc_vector_size(int_vec), oc_vector_capacity(int_vec));

    // Push some integers into it
    for (int i = 0; i < 10; ++i) {
        int val = (i + 1) * 10;
        printf("Pushing %d\n", val);
        oc_vector_push_back(int_vec, &val);
    }

    printf("After pushing 10 elements. Size: %zu, capacity: %zu\n",
           oc_vector_size(int_vec), oc_vector_capacity(int_vec));

    // Read and print the values
    printf("Vector contents:\n");
    for (size_t i = 0; i < oc_vector_size(int_vec); ++i) {
        const int* val_ptr = (const int*)oc_vector_get(int_vec, i);
        if (val_ptr) {
            printf("  Index %zu: %d\n", i, *val_ptr);
        }
    }

    // Clean up
    oc_vector_destroy(int_vec);
    printf("Vector destroyed.\n");

    return 0;
}
