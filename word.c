#include "word.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// Constructor for a word.
//
// syms:    Array of symbols a Word represents.
// len:     Length of the array of symbols.
// returns: Pointer to a Word that has been allocated memory.
//
Word *word_create(uint8_t *syms, uint32_t len) {
  // Allocate memory for word member
  Word *word = (Word *)calloc(1, sizeof(Word));
  if (!word) {
    printf("Error: Failed to allocate memory for word struct!\n");
    exit(EXIT_FAILURE);
  }

  // Allocate memory for symbol
  word->syms = (uint8_t *)calloc(len, sizeof(uint8_t));
  if (!word->syms) {
    printf("Error: Failed to allocate memory for word sym!\n");
    exit(EXIT_FAILURE);
  }

  // Copy the symbol over to the new word member
  memcpy(word->syms, syms, len);
  // Set the length of the symbol to the length being passed
  word->len = len;
  return word;
}

//
// Constructs a new Word from the specified Word appended with a symbol.
// The Word specified to append to may be empty.
// If the above is the case, the new Word should contain only the symbol.
//
// w:       Word to append to.
// sym:     Symbol to append.
// returns: New Word which represents the result of appending.
//
Word *word_append_sym(Word *w, uint8_t sym) {
  // Check is the word being passed has a symbol
  if (w->len) {
    // Allocate a new word member
    Word *word = (Word *)calloc(1, sizeof(Word));
    if (!word) {
      printf("Error: Failed to allocate memory for word struct!\n");
      exit(EXIT_FAILURE);
    }
    // Allocates memory for new symbol with lenght + 1 elements
    word->syms = (uint8_t *)calloc(w->len + 1, sizeof(uint8_t));
    if (!word->syms) {
      printf("Error: Failed to allocate memory for word syms!\n");
      exit(EXIT_FAILURE);
    }
    // Copy the symbol from the old word to the new word
    memcpy(word->syms, w->syms, w->len);
    // Append the symbol to the end of the new word
    word->syms[w->len] = sym;
    // Set the new words length to the old words length +1
    word->len = w->len + 1;
    return word;
  } else {
    // Else condition when there is no previous word and just created
    // a length of 1 word symbol
    Word *word = word_create(&sym, 1);
    return word;
  }
}

//
// Destructor for a Word.
//
// w:       Word to free memory for.
// returns: Void.
//
void word_delete(Word *w) {
  free(w->syms);
  free(w);
  return;
}

//
// Creates a new WordTable, which is an array of Words.
// A WordTable has a pre-defined size of MAX_CODE (UINT16_MAX - 1).
// This is because codes are 16-bit integers.
// A WordTable is initialized with a single Word at index EMPTY_CODE.
// This Word represents the empty word, a string of length of zero.
//
// returns: Initialized WordTable.
//
WordTable *wt_create(void) {
  // Allowed memory for a word table with MAX_CODE elements
  WordTable *wt = (WordTable *)calloc(MAX_CODE, sizeof(Word *));
  if (!wt) {
    printf("Error: Failed to allocate memory for WT!\n");
    exit(EXIT_FAILURE);
  }

  // Allocated memory for the empty word
  Word *word = (Word *)calloc(1, sizeof(Word));
  if (!word) {
    printf("Error: Failed to allocate memory for word struct!\n");
    exit(EXIT_FAILURE);
  }

  word->syms = NULL;
  word->len = 0;
  // Set the index 1/EMPTY_CODE to the empty word
  wt[EMPTY_CODE] = word;
  return wt;
}

//
// Resets a WordTable to having just the empty Word.
//
// wt:      WordTable to reset.
// returns: Void.
//
void wt_reset(WordTable *wt) {
  // Loop through all the element except for the empty word
  // and deallowed all the word within the table
  for (int i = 2; i < MAX_CODE; i++) {
    // Check id the word exists
    if (wt[i]) {
      // Call the destructor to free the words memory
      word_delete(wt[i]);
    }
  }
  return;
}

//
// Deletes an entire WordTable.
// All Words in the WordTable must be deleted as well.
//
// wt:      WordTable to free memory for.
// returns: Void.
//
void wt_delete(WordTable *wt) {
  // Loop through all the words within the table
  for (uint32_t i = 1; i < MAX_CODE; i++) {
    // Check if the word exists
    if (wt[i]) {
      // Call destructor to free memory for word
      word_delete(wt[i]);
    }
  }
  free(wt);
  return;
}
