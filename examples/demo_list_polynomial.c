// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// Assuming omnic/list.h is in the include path
#include <omnic/list.h>

// Represents one term of a polynomial, e.g., 5x^2 would be { coef: 5, exp: 2 }
// It MUST contain an `oc_list_node_t` member for the intrusive list.
typedef struct {
  int coef;             // The coefficient
  int exp;              // The exponent
  oc_list_node_t link;  // The intrusive list node (NOT a pointer)
} poly_t;

// A "dumper" function for printing a single poly_t term.
void print_poly_term(const void* entry, FILE* stream) {
  const poly_t* term = (const poly_t*)entry;
  fprintf(stream, "{ coef: %d, exp: %d }", term->coef, term->exp);
}

// A more user-friendly function to print a full polynomial.
// Example: 3x^2 - 5x + 1
void print_polynomial(const char* title, const oc_list_t* poly) {
  printf("%s", title);
  if (oc_list_empty(poly)) {
    printf("0\n");
    return;
  }

  bool first_term = true;
  poly_t* iter = NULL;
  oc_list_for_each(iter, poly, poly_t, link) {
    if (iter->coef == 0) {
      continue;
    }

    if (!first_term) {
      if (iter->coef > 0) {
        printf(" + ");
      } else {
        printf(" - ");
      }
    } else if (iter->coef < 0) {
      printf("-");
    }

    int abs_coef = iter->coef > 0 ? iter->coef : -iter->coef;

    if (abs_coef != 1 || iter->exp == 0) {
      printf("%d", abs_coef);
    }

    if (iter->exp > 0) {
      printf("x");
      if (iter->exp > 1) {
        printf("^%d", iter->exp);
      }
    }

    first_term = false;
  }
  printf("\n");
}

// --- Core Polynomial Functions ---

/**
 * @brief Adds two polynomials.
 * Assumes input lists poly_a and poly_b are sorted by exponent in descending order.
 * This version is compatible with the provided omnic/list.h API.
 * @return A new oc_list_t representing the sum, which the caller must destroy.
 */
oc_list_t polynomial_add(const oc_list_t* poly_a, const oc_list_t* poly_b) {
  oc_list_t sum_poly;
  oc_list_init(&sum_poly);

  // Get pointers to the first node of each list directly from the list head.
  oc_list_node_t* node_a = poly_a->head;
  oc_list_node_t* node_b = poly_b->head;

  while (node_a != NULL && node_b != NULL) {
    // Use the internal _oc_list_entry macro to get the container struct.
    poly_t* term_a = _oc_list_entry(node_a, poly_t, link);
    poly_t* term_b = _oc_list_entry(node_b, poly_t, link);

    if (term_a->exp > term_b->exp) {
      oc_list_emplace_back(&sum_poly, poly_t, link, .coef = term_a->coef,
                           .exp = term_a->exp);
      node_a = node_a->next;
    } else if (term_b->exp > term_a->exp) {
      oc_list_emplace_back(&sum_poly, poly_t, link, .coef = term_b->coef,
                           .exp = term_b->exp);
      node_b = node_b->next;
    } else {  // Exponents are equal
      int new_coef = term_a->coef + term_b->coef;
      if (new_coef != 0) {
        oc_list_emplace_back(&sum_poly, poly_t, link, .coef = new_coef,
                             .exp = term_a->exp);
      }
      node_a = node_a->next;
      node_b = node_b->next;
    }
  }

  // Add any remaining terms from poly_a
  while (node_a != NULL) {
    poly_t* term_a = _oc_list_entry(node_a, poly_t, link);
    oc_list_emplace_back(&sum_poly, poly_t, link, .coef = term_a->coef,
                         .exp = term_a->exp);
    node_a = node_a->next;
  }

  // Add any remaining terms from poly_b
  while (node_b != NULL) {
    poly_t* term_b = _oc_list_entry(node_b, poly_t, link);
    oc_list_emplace_back(&sum_poly, poly_t, link, .coef = term_b->coef,
                         .exp = term_b->exp);
    node_b = node_b->next;
  }

  return sum_poly;
}

/**
 * @brief Multiplies two polynomials.
 * This version is compatible with the provided omnic/list.h API, which lacks
 * a mid-list insertion function. It works by repeatedly using polynomial_add.
 * @return A new oc_list_t representing the product,
 *         which the caller must destroy.
 */
oc_list_t polynomial_multiply(const oc_list_t* poly_a,
                              const oc_list_t* poly_b) {
  oc_list_t total_prod_poly;
  oc_list_init(&total_prod_poly);

  poly_t* term_a;
  // For each term in polynomial A...
  oc_list_for_each(term_a, poly_a, poly_t, link) {
    oc_list_t partial_prod;
    oc_list_init(&partial_prod);

    // ...multiply it by every term in polynomial B to get a partial product list
    poly_t* term_b;
    oc_list_for_each(term_b, poly_b, poly_t, link) {
      int new_coef = term_a->coef * term_b->coef;
      int new_exp = term_a->exp + term_b->exp;
      if (new_coef != 0) {
        oc_list_emplace_back(&partial_prod, poly_t, link, .coef = new_coef,
                             .exp = new_exp);
      }
    }

    // Now add this partial product to our running total
    oc_list_t new_sum = polynomial_add(&total_prod_poly, &partial_prod);

    // Clean up the old total and the partial product list
    oc_list_destroy(&total_prod_poly, poly_t, link);
    oc_list_destroy(&partial_prod, poly_t, link);

    // The new sum becomes our new running total
    total_prod_poly = new_sum;
  }

  return total_prod_poly;
}

int main(void) {
  printf("--- OmniC Intrusive List-based Polynomial Example ---\n\n");

  oc_list_t poly_a;
  oc_list_t poly_b;

  oc_list_init(&poly_a);
  oc_list_init(&poly_b);

  // Populate Polynomial A: 3x^2 - 5x + 1
  // IMPORTANT: Add terms in descending order of exponent to keep the list
  // sorted.
  // TODO: Otherwise, implement a fast sort algo to do this.
  printf("Creating Polynomial A: 3x^2 - 5x + 1\n");
  oc_list_emplace_back(&poly_a, poly_t, link, .coef = 3, .exp = 2);
  oc_list_emplace_back(&poly_a, poly_t, link, .coef = -5, .exp = 1);
  oc_list_emplace_back(&poly_a, poly_t, link, .coef = 1, .exp = 0);

  // Populate Polynomial B: 2x^3 + 4x
  printf("Creating Polynomial B: 2x^3 + 4x\n\n");
  oc_list_emplace_back(&poly_b, poly_t, link, .coef = 2, .exp = 3);
  oc_list_emplace_back(&poly_b, poly_t, link, .coef = 4, .exp = 1);

  print_polynomial("Poly A: ", &poly_a);
  print_polynomial("Poly B: ", &poly_b);
  printf("\n");

  // --- Addition ---
  printf("--- Calculating Sum ---\n");
  oc_list_t poly_sum = polynomial_add(&poly_a, &poly_b);
  print_polynomial("Sum   : ", &poly_sum);
  printf("Expected: 2x^3 + 3x^2 - x + 1\n\n");

  // --- Multiplication ---
  printf("--- Calculating Product ---\n");
  oc_list_t poly_prod = polynomial_multiply(&poly_a, &poly_b);
  print_polynomial("Product : ", &poly_prod);
  printf("Expected: 6x^5 - 10x^4 + 14x^3 - 20x^2 + 4x\n\n");

  // --- Cleanup ---
  printf("Destroying all lists to free memory...\n");
  oc_list_destroy(&poly_a, poly_t, link);
  oc_list_destroy(&poly_b, poly_t, link);
  oc_list_destroy(&poly_sum, poly_t, link);
  oc_list_destroy(&poly_prod, poly_t, link);
  printf("Done.\n");

  return 0;
}
