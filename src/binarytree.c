// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2; -*-

#include <assert.h>
#include <omnic/binarytree.h>
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/* --- Core API Implementation --- */
/* -------------------------------------------------------------------------- */

oc_bintree_node_t* oc_bintree_create_node(void* data) {
  // Use calloc to initialize memory to zero/NULL
  oc_bintree_node_t* new_node =
      (oc_bintree_node_t*)calloc(1, sizeof(oc_bintree_node_t));
  if (new_node == NULL) {
    fprintf(stderr, "[OmniC][BinTree] Error: Failed to allocate new node.\n");
    return NULL;
  }
  new_node->data = data;
  return new_node;
}

void oc_bintree_destroy(oc_bintree_node_t* root, oc_bintree_data_dtor_t dtor) {
  if (root == NULL) {
    return;
  }

  // Post-order traversal for deletion
  oc_bintree_destroy(root->left, dtor);
  oc_bintree_destroy(root->right, dtor);

  // Free the user's data payload if a destructor is provided
  if (dtor && root->data) {
    dtor(root->data);
  }

  // Free the node structure itself
  free(root);
}

void oc_bintree_set_left(oc_bintree_node_t* parent, oc_bintree_node_t* child) {
  assert(parent != NULL && "[OmniC][BinTree] Parent node cannot be NULL.");
  parent->left = child;
}

void oc_bintree_set_right(oc_bintree_node_t* parent, oc_bintree_node_t* child) {
  assert(parent != NULL && "[OmniC][BinTree] Parent node cannot be NULL.");
  parent->right = child;
}

/* -------------------------------------------------------------------------- */
/* --- Traversal Implementations --- */
/* -------------------------------------------------------------------------- */

void _oc_bintree_traverse_preorder(oc_bintree_node_t* node,
                                   oc_bintree_traverser_t traverser) {
  if (node == NULL || traverser == NULL) {
    return;
  }

  // Visit the root
  traverser(node->data);
  // Traverse left subtree
  _oc_bintree_traverse_preorder(node->left, traverser);
  // Traverse right subtree
  _oc_bintree_traverse_preorder(node->right, traverser);
}

void _oc_bintree_traverse_inorder(oc_bintree_node_t* node,
                                  oc_bintree_traverser_t traverser) {
  if (node == NULL || traverser == NULL) {
    return;
  }

  // Traverse left subtree
  _oc_bintree_traverse_inorder(node->left, traverser);
  // Visit the root
  traverser(node->data);
  // Traverse right subtree
  _oc_bintree_traverse_inorder(node->right, traverser);
}

void _oc_bintree_traverse_postorder(oc_bintree_node_t* node,
                                    oc_bintree_traverser_t traverser) {
  if (node == NULL || traverser == NULL) {
    return;
  }

  // Traverse left subtree
  _oc_bintree_traverse_postorder(node->left, traverser);
  // Traverse right subtree
  _oc_bintree_traverse_postorder(node->right, traverser);
  // Visit the root
  traverser(node->data);
}

/* -------------------------------------------------------------------------- */
/* --- Calculation Implementations --- */
/* -------------------------------------------------------------------------- */

size_t _oc_bintree_count_nodes(oc_bintree_node_t* node) {
  if (node == NULL) {
    return 0;
  }
  // 1 (current node) + nodes in left subtree + nodes in right subtree
  return 1 + _oc_bintree_count_nodes(node->left) +
         _oc_bintree_count_nodes(node->right);
}

size_t _oc_bintree_count_leaves(oc_bintree_node_t* node) {
  if (node == NULL) {
    return 0;
  }
  // A node is a leaf if it has no children
  if (node->left == NULL && node->right == NULL) {
    return 1;
  }
  // Recursively sum leaves from both subtrees
  return _oc_bintree_count_leaves(node->left) +
         _oc_bintree_count_leaves(node->right);
}

size_t _oc_bintree_get_height(oc_bintree_node_t* node) {
  if (node == NULL) {
    return 0;  // Height of an empty tree is 0
  }
  // Height is 1 + maximum height of left or right subtree
  size_t left_height = _oc_bintree_get_height(node->left);
  size_t right_height = _oc_bintree_get_height(node->right);

  // Standard C does not have a native max, so we use a conditional
  return 1 + (left_height > right_height ? left_height : right_height);
}

/* -------------------------------------------------------------------------- */
/* --- Mirror Implementation --- */
/* -------------------------------------------------------------------------- */

void oc_bintree_mirror(oc_bintree_node_t* root) {
  if (root == NULL) {
    return;
  }

  // Recursively mirror subtrees
  oc_bintree_mirror(root->left);
  oc_bintree_mirror(root->right);

  // Swap the children
  oc_bintree_node_t* temp = root->left;
  root->left = root->right;
  root->right = temp;
}
