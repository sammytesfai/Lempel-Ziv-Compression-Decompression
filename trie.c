#include "trie.h"
#include <stdio.h>
#include <stdlib.h>

//
// Constructor for a TrieNode.
//
// index:   Index of the constructed TrieNode.
// returns: Pointer to a TrieNode that has been allocated memory.
//
TrieNode *trie_node_create(uint16_t index) {
  TrieNode *new_node = (TrieNode *)calloc(1, sizeof(TrieNode));

  // Error check if memory allocation failed
  if (!new_node) {
    printf("Error: Failed to allocate memory for TrieNode!\n");
    exit(EXIT_FAILURE);
  }

  // Set the code for the node
  new_node->code = index;
  return new_node;
}

//
// Destructor for a TrieNode.
//
// n:       TrieNode to free allocated memory for.
// returns: Void.
//
void trie_node_delete(TrieNode *n) {
  free(n);
  return;
}

//
// Initializes a Trie: a root TrieNode with the index EMPTY_CODE.
//
// returns: Pointer to the root of a Trie.
//
TrieNode *trie_create(void) {
  // Allocate memory for the root node
  TrieNode *root = trie_node_create(EMPTY_CODE);
  return root;
}

//
// Resets a Trie to just the root TrieNode.
//
// root:    Root of the Trie to reset.
// returns: Void.
//
void trie_reset(TrieNode *root) {
  // Loop to reset all the children the in node
  for (int i = 0; i < ALPHABET; i++) {
    // Check if the child even exists
    if (root->children[i]) {
      // Call destructor to deallocate memory for child and all of its
      // children
      trie_delete(root->children[i]);
      // Set the child node to null
      root->children[i] = NULL;
    }
  }
  return;
}

//
// Deletes a sub-Trie starting from the sub-Trie's root.
//
// n:       Root of the sub-Trie to delete.
// returns: Void.
//
void trie_delete(TrieNode *n) {
  //Check if node being passed exists
  if (n) {
    // Loop to deallocate memory for all the children in node
    for (int i = 0; i < ALPHABET; i++) {
      // recursively call itself to deallocate the memory of the childrens
      // children
      trie_delete(n->children[i]);
    }
    // Call destructor to free memory
    trie_node_delete(n);
    n = NULL;
  }
  return;
}

//
// Returns a pointer to the child TrieNode reprsenting the symbol sym.
// If the symbol doesn't exist, NULL is returned.
//
// n:       TrieNode to step from.
// sym:     Symbol to check for.
// returns: Pointer to the TrieNode representing the symbol.
//
TrieNode *trie_step(TrieNode *n, uint8_t sym) {
  return n->children[sym];
}
