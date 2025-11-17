// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#ifndef OMNIC_HUFFMAN_H
#define OMNIC_HUFFMAN_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>  // For malloc, free

/* -------------------------------------------------------------------------- */

/// @file huffman.h
/// @brief Implementation of Huffman Coding for text compression.
///
/// This module provides functions to build a Huffman tree from a frequency
/// map, encode and decode a text stream, and manage the tree memory.
///
/// **Note:** This implementation uses a simplified array for the priority
/// queue simulation. For a large-scale project, a proper min-heap should
/// be implemented for O(log n) performance on node insertion/extraction.

/* -------------------------------------------------------------------------- */

#define HUFFMAN_CODE_TABLE_SIZE 256

// --- Type Definitions ---

/// @brief Node structure for the Huffman Tree.
typedef struct huffman_node {
  uint8_t symbol;              ///< The character symbol (0-255).
  size_t frequency;            ///< The frequency of the symbol.
  struct huffman_node* left;   ///< Left child node.
  struct huffman_node* right;  ///< Right child node.
} huffman_node_t;

/// @brief Stores the Huffman code for a symbol (up to 256 bits for simplicity).
typedef struct huffman_code {
  ///< The actual binary code string (e.g., "0101").
  char bits[HUFFMAN_CODE_TABLE_SIZE];
  ///< The length of the code in bits.
  size_t length;
} huffman_code_t;

/// @brief Stores the full mapping of codes for all 256 possible symbols.
typedef huffman_code_t huffman_code_table_t[HUFFMAN_CODE_TABLE_SIZE];

/* -------------------------------------------------------------------------- */

// --- Core API Functions ---

/// @brief Builds a Huffman Tree from a given frequency map.
/// @param frequencies An array of 256 size_t values representing the count
///                    of each byte (0-255).
/// @return The root of the constructed Huffman Tree, or NULL if all frequencies
/// are zero.
huffman_node_t* oc_huffman_build_tree(
    const size_t frequencies[HUFFMAN_CODE_TABLE_SIZE]);

/// @brief Destroys the Huffman Tree, freeing all node structures.
/// @param root The root node of the Huffman Tree.
void oc_huffman_destroy_tree(huffman_node_t* root);

/// @brief Generates the canonical Huffman code table from the tree.
/// @param root The root of the Huffman Tree.
/// @param code_table A pointer to the 256-entry code table to populate.
void oc_huffman_build_code_table(const huffman_node_t* root,
                                 huffman_code_table_t code_table);

/// @brief Encodes the input buffer using the provided code table.
/// @param input The raw input data buffer.
/// @param input_len The length of the input buffer.
/// @param code_table The pre-built Huffman code table.
/// @param output A pointer to a newly allocated buffer holding encoded bits.
///               Caller must free this memory.
/// @param output_len The length of the allocated output buffer (in bytes).
/// @return The total number of bits in the encoded data.
size_t oc_huffman_encode(const uint8_t* input, size_t input_len,
                         const huffman_code_table_t code_table,
                         uint8_t** output, size_t* output_len);

/// @brief Decodes the bit-stream buffer using the Huffman Tree.
/// @param input The encoded bit-stream buffer.
/// @param input_bits_len The total length of the encoded data in bits.
/// @param tree_root The root of the Huffman Tree.
/// @param output A pointer to a newly allocated buffer holding decoded data.
///               Caller must free this memory.
/// @param output_len The length of the decoded data (in bytes).
/// @return True on success, false on error (e.g., invalid bit sequence).
bool oc_huffman_decode(const uint8_t* input, size_t input_bits_len,
                       const huffman_node_t* tree_root, uint8_t** output,
                       size_t* output_len);

/* -------------------------------------------------------------------------- */

#endif  // OMNIC_HUFFMAN_H
