// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

// Assuming your header is in omnic/huffmantree.h relative to include paths
#include <omnic/huffmantree.h>

/* -------------------------------------------------------------------------- */
/* --- Internal Structures and Functions --- */
/* -------------------------------------------------------------------------- */

/// @brief Allocates and initializes a new Huffman node.
static huffman_node_t* huffman_create_node(uint8_t symbol, size_t freq,
                                           huffman_node_t* left,
                                           huffman_node_t* right) {
  huffman_node_t* new_node = (huffman_node_t*)calloc(1, sizeof(huffman_node_t));
  if (new_node == NULL) {
    fprintf(stderr, "[OmniC][Huffman] Error: Failed to allocate new node.\n");
    return NULL;
  }
  new_node->symbol = symbol;
  new_node->frequency = freq;
  new_node->left = left;
  new_node->right = right;
  return new_node;
}

// --- Simplified Priority Queue Simulation ---

// Max number of symbols (256) + internal nodes (255)
#define MAX_NODES 511

// An array to simulate a min-priority queue (sorted by frequency)
typedef struct {
  huffman_node_t* nodes[MAX_NODES];
  size_t size;
} min_pq_t;

/// @brief Initializes the priority queue.
static void pq_init(min_pq_t* pq) { pq->size = 0; }

/// @brief Inserts a node while maintaining sorted order (low freq first).
static bool pq_insert(min_pq_t* pq, huffman_node_t* node) {
  if (pq->size >= MAX_NODES) {
    return false;  // Should never happen in a valid Huffman run
  }

  // Find the insertion point to maintain ascending order by frequency
  size_t i = pq->size;
  while (i > 0 && pq->nodes[i - 1]->frequency > node->frequency) {
    pq->nodes[i] = pq->nodes[i - 1];
    i--;
  }

  pq->nodes[i] = node;
  pq->size++;
  return true;
}

/// @brief Extracts the node with the minimum frequency (the first element).
static huffman_node_t* pq_extract_min(min_pq_t* pq) {
  if (pq->size == 0) {
    return NULL;
  }
  huffman_node_t* min_node = pq->nodes[0];
  // Shift all elements
  for (size_t i = 0; i < pq->size - 1; i++) {
    pq->nodes[i] = pq->nodes[i + 1];
  }
  pq->size--;
  return min_node;
}

// --- Recursive Code Table Builder ---

/// @brief Internal function to recursively traverse the tree and fill the code
/// table.
static void huffman_build_codes_recursive(
    const huffman_node_t* node, char current_code[HUFFMAN_CODE_TABLE_SIZE],
    size_t depth, huffman_code_table_t code_table) {
  if (node == NULL) {
    return;
  }

  // Leaf node: we found a symbol and its code
  if (node->left == NULL && node->right == NULL) {
    // Copy the code and set the length
    memcpy(code_table[node->symbol].bits, current_code, depth);
    code_table[node->symbol].bits[depth] = '\0';  // Null-terminate
    code_table[node->symbol].length = depth;
    return;
  }

  // Traverse left (append '0')
  current_code[depth] = '0';
  huffman_build_codes_recursive(node->left, current_code, depth + 1,
                                code_table);

  // Traverse right (append '1')
  current_code[depth] = '1';
  huffman_build_codes_recursive(node->right, current_code, depth + 1,
                                code_table);
}

/* -------------------------------------------------------------------------- */
/* --- Public API Implementation --- */
/* -------------------------------------------------------------------------- */

huffman_node_t* oc_huffman_build_tree(
    const size_t frequencies[HUFFMAN_CODE_TABLE_SIZE]) {
  min_pq_t pq;
  pq_init(&pq);

  // Create a leaf node for every symbol with a non-zero frequency
  for (int i = 0; i < 256; i++) {
    if (frequencies[i] > 0) {
      huffman_node_t* node = huffman_create_node(i, frequencies[i], NULL, NULL);
      if (node == NULL || !pq_insert(&pq, node)) {
        // Simple error handling: destroy all nodes created so far and return
        fprintf(stderr,
                "[OmniC][Huffman] Error during initial node creation.\n");
        // We'll skip cleanup here for conciseness
        while (pq.size > 0) {
          free(pq_extract_min(&pq));
        }
        return NULL;
      }
    }
  }

  // Handle edge cases after populating the queue
  if (pq.size == 0) {
    return NULL;  // Empty input resulted in an empty queue
  }

  if (pq.size == 1) {
    // If there's only one symbol, create a parent node for it. This forms a
    // valid tree and gives the symbol a code of length 1 (e.g., "0").
    huffman_node_t* single_node = pq_extract_min(&pq);
    huffman_node_t* dummy_root =
        huffman_create_node(0, single_node->frequency, single_node, NULL);

    if (dummy_root == NULL) {
      free(single_node);  // Don't leak the node
      return NULL;
    }
    return dummy_root;
  }

  // Build the tree by repeatedly merging the two lowest-frequency nodes
  while (pq.size > 1) {
    huffman_node_t* left = pq_extract_min(&pq);
    huffman_node_t* right = pq_extract_min(&pq);

    size_t new_freq = left->frequency + right->frequency;
    // The symbol for an internal node doesn't matter, use 0
    huffman_node_t* parent = huffman_create_node(0, new_freq, left, right);

    if (parent == NULL || !pq_insert(&pq, parent)) {
      // Error in internal node creation. Cleanup is complex, returning NULL
      fprintf(stderr, "[OmniC][Huffman] Error creating internal node.\n");
      // A proper cleanup of all partial nodes is required for production code.
      free(left);
      free(right);
      free(parent);
      while (pq.size > 0) {
        oc_huffman_destroy_tree(pq_extract_min(&pq));
      }
      return NULL;
    }
  }

  // The last remaining node is the root of the Huffman Tree
  return pq_extract_min(&pq);
}

void oc_huffman_destroy_tree(huffman_node_t* root) {
  if (root == NULL) {
    return;
  }
  // Post-order traversal for deletion
  oc_huffman_destroy_tree(root->left);
  oc_huffman_destroy_tree(root->right);
  free(root);
}

void oc_huffman_build_code_table(const huffman_node_t* root,
                                 huffman_code_table_t code_table) {
  if (root == NULL) {
    return;
  }

  // Initialize all codes to indicate they are unused
  for (int i = 0; i < HUFFMAN_CODE_TABLE_SIZE; i++) {
    code_table[i].length = 0;
    code_table[i].bits[0] = '\0';
  }

  char current_code_buffer[HUFFMAN_CODE_TABLE_SIZE];
  huffman_build_codes_recursive(root, current_code_buffer, 0, code_table);
}

size_t oc_huffman_encode(const uint8_t* input, size_t input_len,
                         const huffman_code_table_t code_table,
                         uint8_t** output, size_t* output_len) {
  if (input == NULL || input_len == 0 || output == NULL || output_len == NULL) {
    if (output)
      *output = NULL;
    if (output_len)
      *output_len = 0;
    return 0;
  }

  // 1. Calculate total bits needed
  size_t total_bits = 0;
  for (size_t i = 0; i < input_len; i++) {
    total_bits += code_table[input[i]].length;
  }

  if (total_bits == 0) {
    *output = NULL;
    *output_len = 0;
    return 0;
  }

  // 2. Allocate output buffer
  size_t total_bytes = (total_bits + 7) / 8;
  *output = (uint8_t*)calloc(total_bytes, sizeof(uint8_t));
  if (*output == NULL) {
    *output_len = 0;
    return 0;
  }
  *output_len = total_bytes;

  // 3. Perform the encoding (bit-packing)
  size_t current_bit_index = 0;

  for (size_t i = 0; i < input_len; i++) {
    uint8_t symbol = input[i];
    const huffman_code_t* code = &code_table[symbol];

    for (size_t j = 0; j < code->length; j++) {
      size_t byte_index = current_bit_index / 8;
      size_t bit_pos = current_bit_index % 8;

      if (code->bits[j] == '1') {
        (*output)[byte_index] |= (1 << bit_pos);  // LSB-first
      }
      current_bit_index++;
    }
  }

  return total_bits;
}

bool oc_huffman_decode(const uint8_t* input, size_t input_bits_len,
                       const huffman_node_t* tree_root, uint8_t** output,
                       size_t* output_len) {
  if (output == NULL || output_len == NULL) {
    return false;  // Invalid parameters
  }

  if (input == NULL || input_bits_len == 0 || tree_root == NULL) {
    *output_len = 0;
    *output = NULL;
    // Decoding nothing is considered a success. If tree_root is NULL but there
    // are bits, that's an issue.
    return (input_bits_len == 0);
  }

  // Estimate initial output size; it can grow if needed.
  size_t max_output_size = input_bits_len;
  if (max_output_size == 0)
    max_output_size = 1;  // Handle tiny inputs
  uint8_t* decoded_data = (uint8_t*)malloc(max_output_size);
  if (decoded_data == NULL) {
    *output_len = 0;
    *output = NULL;
    return false;
  }

  const huffman_node_t* current_node = tree_root;
  size_t decoded_count = 0;

  for (size_t i = 0; i < input_bits_len; i++) {
    size_t byte_index = i / 8;
    size_t bit_pos = i % 8;
    int bit = (input[byte_index] >> bit_pos) & 1;

    current_node = (bit == 0) ? current_node->left : current_node->right;

    if (current_node == NULL) {
      fprintf(stderr,
              "[OmniC][Huffman] Error: Invalid bit sequence during decode.\n");
      free(decoded_data);
      *output_len = 0;
      *output = NULL;
      return false;
    }

    if (current_node->left == NULL && current_node->right == NULL) {
      if (decoded_count >= max_output_size) {
        max_output_size *= 2;
        uint8_t* new_data = (uint8_t*)realloc(decoded_data, max_output_size);
        if (new_data == NULL) {
          free(decoded_data);
          *output_len = 0;
          *output = NULL;
          return false;
        }
        decoded_data = new_data;
      }
      decoded_data[decoded_count++] = current_node->symbol;
      current_node = tree_root;  // Reset for the next symbol
    }
  }

  if (current_node != tree_root && input_bits_len > 0) {
    fprintf(stderr,
            "[OmniC][Huffman] Warning: Ended decode inside a non-leaf node. "
            "Possible truncated input.\n");
  }

  // Shrink buffer to fit exactly
  uint8_t* final_data = (uint8_t*)realloc(decoded_data, decoded_count);
  *output =
      (final_data != NULL || decoded_count == 0) ? final_data : decoded_data;
  *output_len = decoded_count;
  return true;
}
