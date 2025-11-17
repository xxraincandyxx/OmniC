// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#include <omnic/huffmantree.h>  // Includes the Huffman API
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
// --- Test Framework Setup (Copied from binarytree_test.c) ---
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
/* --- Test Functions --- */
/* -------------------------------------------------------------------------- */

/// @brief Helper to generate frequencies from a string.
static void generate_frequencies(const char* input,
                                 size_t frequencies[HUFFMAN_CODE_TABLE_SIZE]) {
  memset(frequencies, 0, HUFFMAN_CODE_TABLE_SIZE * sizeof(size_t));
  for (size_t i = 0; input[i] != '\0'; i++) {
    frequencies[(uint8_t)input[i]]++;
  }
}

void test_tree_creation_and_codes() {
  printf("--- Testing Tree Creation and Code Table Generation ---\n");

  const char* text = "BCAADDDCCACACAC";
  size_t frequencies[HUFFMAN_CODE_TABLE_SIZE];
  generate_frequencies(text, frequencies);

  // Correct Frequencies: C=6, A=5, D=3, B=1
  // A valid Huffman tree for these frequencies should produce these lengths.
  // The exact codes ("0", "101") may vary, but lengths are predictable.
  // C: 1 bit (highest frequency)
  // A: 2 bits
  // D: 3 bits
  // B: 3 bits (lowest frequency)

  huffman_node_t* root = oc_huffman_build_tree(frequencies);
  ASSERT(root != NULL, "Huffman Tree root creation successful");

  if (root == NULL)
    return;  // Guard against further errors if creation fails

  huffman_code_table_t codes;
  oc_huffman_build_code_table(root, codes);

  ASSERT_EQ(codes['C'].length, (size_t)1, "%zu", "Code length for 'C' is 1");
  ASSERT_EQ(codes['A'].length, (size_t)2, "%zu", "Code length for 'A' is 2");
  ASSERT_EQ(codes['D'].length, (size_t)3, "%zu", "Code length for 'D' is 3");
  ASSERT_EQ(codes['B'].length, (size_t)3, "%zu", "Code length for 'B' is 3");
  ASSERT_EQ(codes['X'].length, (size_t)0, "%zu",
            "Code length for unused 'X' is 0");

  oc_huffman_destroy_tree(root);
  printf("\n");
}

void test_encoding_decoding() {
  printf("--- Testing Encoding and Decoding End-to-End ---\n");

  const char* original_text = "Mississippi river";
  size_t original_len = strlen(original_text);
  size_t frequencies[HUFFMAN_CODE_TABLE_SIZE];
  generate_frequencies(original_text, frequencies);

  huffman_node_t* root = oc_huffman_build_tree(frequencies);
  ASSERT(root != NULL, "End-to-end test: Tree created");
  if (root == NULL)
    return;

  huffman_code_table_t codes;
  oc_huffman_build_code_table(root, codes);

  // --- Encoding Test ---
  uint8_t* encoded_output = NULL;
  size_t encoded_bytes_len = 0;
  size_t total_bits =
      oc_huffman_encode((const uint8_t*)original_text, original_len, codes,
                        &encoded_output, &encoded_bytes_len);

  ASSERT(encoded_output != NULL, "Encoding output buffer allocated");
  ASSERT(total_bits > 0 && total_bits < (original_len * 8),
         "Total bits calculated correctly (compression achieved)");

  size_t expected_bytes = (total_bits + 7) / 8;
  ASSERT_EQ(encoded_bytes_len, expected_bytes, "%zu",
            "Encoded output size in bytes matches calculated size");

  // --- Decoding Test ---
  uint8_t* decoded_output = NULL;
  size_t decoded_len = 0;

  bool decode_success = oc_huffman_decode(encoded_output, total_bits, root,
                                          &decoded_output, &decoded_len);

  ASSERT(decode_success, "Decoding process completed successfully");
  ASSERT(decoded_output != NULL, "Decoding output buffer allocated");
  ASSERT_EQ(decoded_len, original_len, "%zu",
            "Decoded length matches original length");

  bool content_match =
      (decoded_output != NULL) &&
      (memcmp(original_text, decoded_output, original_len) == 0);
  ASSERT(content_match, "Decoded content matches original text");

  // --- Cleanup ---
  free(encoded_output);
  free(decoded_output);
  oc_huffman_destroy_tree(root);
  printf("\n");
}

void test_edge_cases() {
  printf("--- Testing Huffman Edge Cases ---\n");

  // --- Edge Case 1: Empty Input ---
  size_t frequencies_empty[HUFFMAN_CODE_TABLE_SIZE] = {0};
  huffman_node_t* root_empty = oc_huffman_build_tree(frequencies_empty);
  ASSERT(root_empty == NULL, "Empty input yields a NULL tree");
  oc_huffman_destroy_tree(root_empty);

  // --- Edge Case 2: Single Symbol Input ---
  const char* single_char_text = "AAAAA";
  size_t text_len = strlen(single_char_text);
  size_t frequencies_single[HUFFMAN_CODE_TABLE_SIZE];
  generate_frequencies(single_char_text, frequencies_single);

  huffman_node_t* root_single = oc_huffman_build_tree(frequencies_single);
  ASSERT(root_single != NULL, "Single symbol input yields a non-NULL root");
  if (!root_single)
    return;

  huffman_code_table_t codes_single;
  oc_huffman_build_code_table(root_single, codes_single);

  ASSERT(codes_single['A'].length == (size_t)1,
         "Single symbol code has length 1");

  uint8_t* encoded_single = NULL;
  size_t encoded_bytes_single = 0;
  size_t total_bits_single =
      oc_huffman_encode((const uint8_t*)single_char_text, text_len,
                        codes_single, &encoded_single, &encoded_bytes_single);

  ASSERT_EQ(total_bits_single, codes_single['A'].length * text_len, "%zu",
            "Single symbol encoding has correct total bits");

  uint8_t* decoded_single = NULL;
  size_t decoded_len_single = 0;
  bool decode_ok =
      oc_huffman_decode(encoded_single, total_bits_single, root_single,
                        &decoded_single, &decoded_len_single);

  ASSERT(decode_ok, "Single symbol decode successful");
  ASSERT_EQ(decoded_len_single, text_len, "%zu",
            "Single symbol decode length correct");
  // FIX: Added a NULL check to prevent segfault
  ASSERT(decoded_single != NULL &&
             memcmp(decoded_single, single_char_text, text_len) == 0,
         "Single symbol decode content correct");

  free(encoded_single);
  free(decoded_single);
  oc_huffman_destroy_tree(root_single);
  printf("\n");
}

/* -------------------------------------------------------------------------- */
// --- Main Test Runner ---
/* -------------------------------------------------------------------------- */

int main(void) {
  printf("--- Running OmniC Huffman Tree Test Suite ---\n\n");

  test_tree_creation_and_codes();
  test_encoding_decoding();
  test_edge_cases();

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
