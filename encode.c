#include "bv.h"
#include "code.h"
#include "io.h"
#include "trie.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

// Defined option for the command line arguements
#define OPTIONS "vi:o:"

// Global variables to count bytes
// for compression and decompression
uint64_t total_syms;
uint64_t total_bits;

// Bool flags for getopt arguments
bool Stats = false;
bool user_infile = false;
bool user_outfile = false;

// Global Bitbuffer to be accessed in io.c
BitVector *bitbuf;

//
// This function simply identifies the minimum number
// of bits needed for the code being passsed in
//
// uint16_t nex_code:		16 bit code integer
//
uint8_t bit_length(uint16_t next_code);

//
// This function parses through the command line arguments
// and sets the appropriate flags for the program
//
// int argc:		The number of command line arguements to parse through
// char **argv:		Char pointer holding all the arguments
// char **read_file:	Passed by refernce char pointer for infile name
// char **write_file:	PAssed by reference char pointer for outfile name
//
void get_options(int argc, char **argv, char **read_file, char **write_file);

int main(int argc, char **argv) {
  // nitialize char pointers for files names
  char *read_file = NULL;
  char *write_file = NULL;

  // Call the get_options functions to get the command line arguments and
  // set the appropriate flags
  get_options(argc, argv, &read_file, &write_file);

  // Allocate memory for a FileHeader member
  FileHeader *header = (FileHeader *)calloc(1, sizeof(FileHeader));
  if (!header) {
    printf("Error: Failed to allocated memory for FileHeader!\n");
    exit(EXIT_FAILURE);
  }

  // Initialize the magic number in the File Header
  header->magic = MAGIC;

  // Create a bit buffer will 4096 elements
  bitbuf = bv_create(BLOCK * 8);

  // In and outfile descriptors
  int infile = 0;
  int outfile = 0;

  // Set the infile to the read in file from command line arguments
  // or set it to STDIN by default
  if (user_infile) {
    infile = open(read_file, O_RDONLY);
  } else {
    infile = STDIN_FILENO;
  }

  // Set the outfile to the read in file from command line arguments
  // or set it to STDOUT by default
  if (user_outfile) {
    outfile = open(write_file, O_WRONLY | O_CREAT | O_TRUNC);
  } else {
    outfile = STDOUT_FILENO;
  }

  // Get the protection number from the infile and copy it over to the
  // outfile
  struct stat srcstats;
  fstat(infile, &srcstats);
  fchmod(outfile, srcstats.st_mode);
  header->protection = srcstats.st_mode;

  // write the haeader file into the outfile
  write_header(outfile, header);

  // Declare Tre root and helper pointers
  TrieNode *root = trie_create();
  TrieNode *curr_node = root;
  TrieNode *prev_node = NULL;

  // Declare helper symbol variables
  uint8_t curr_sym = 0;
  uint8_t prev_sym = 0;

  // Setting the next_code to the start of the code (2)
  uint16_t next_code = START_CODE;

  // Loop until there is no symbols left to process
  while (read_sym(infile, &curr_sym)) {
    // Set the next_node to the child of the current node based on the current
    // symbol as the index
    TrieNode *next_node = trie_step(curr_node, curr_sym);

    // Check if that node exists
    if (next_node != NULL) {
      prev_node = curr_node;
      curr_node = next_node;
    } else {
      // Buffer the current symbol into the write buffer with its corresponding
      // code
      buffer_pair(outfile, curr_node->code, curr_sym, bit_length(next_code));
      curr_node->children[curr_sym] = trie_node_create(next_code);
      curr_node = root;
      next_code = next_code + 1;
    }
    // Check if the code is at the MAX of a uint16
    if (next_code == MAX_CODE) {
      // If so reset the Trie ADT and reset the node and code
      trie_reset(root);
      curr_node = root;
      next_code = START_CODE;
    }
    prev_sym = curr_sym;
  }
  if (curr_node != root) {
    buffer_pair(outfile, prev_node->code, prev_sym, bit_length(next_code));
    next_code = (next_code + 1) % MAX_CODE;
  }

  // Put the STOP_CODE value with no symbol to signify the end of the buffer/file
  buffer_pair(outfile, STOP_CODE, 0, bit_length(next_code));
  // Flush any remaining bits from the buffer into the oufile
  flush_pairs(outfile);

  // If the infile isnt STDIN close the file descriptor
  if (user_infile) {
    if (close(infile) < 0) {
      printf("Error: Failed to close infile!\n");
      exit(EXIT_FAILURE);
    }
  }
  // If the outfile isnt STDOUT close the file descriptor
  if (user_outfile) {
    if (close(outfile) < 0) {
      printf("Error: Failed to close outfile!\n");
      exit(EXIT_FAILURE);
    }
  }

  // Stats calculation and output if flag is set in command line arguements
  uint64_t compressed = 0;
  if (total_bits % 8 == 0) {
    compressed = total_bits / 8;
  } else {
    compressed = total_bits / 8 + 1;
  }
  if (Stats) {
    printf("Compressed file size: %lu bytes\n", compressed);
    printf("Uncompressed file size: %lu bytes\n", total_syms);
    printf("Compressed ratio: %.2lf%%\n",
        100 * (1 - (compressed / 1.00) / total_syms));
  }

  // Deallocate memory from Trie ADT, bit buffer, and File Header
  trie_delete(root);
  bv_delete(bitbuf);
  free(header);
  return 0;
}

//
// This function simply identifies the minimum number
// of bits needed for the code being passsed in
//
// uint16_t nex_code:           16 bit code integer
//
uint8_t bit_length(uint16_t next_code) {
  uint8_t zeros = 0;
  uint8_t bits = 0;
  // Loop through all the bits in the code
  for (int i = 0; i < 16; i++) {
    zeros++;
    // Check if each is on
    if (next_code & (1 << i)) {
      // Add the place of the bit
      bits = zeros;
    }
  }
  return bits;
}

//
// This function parses through the command line arguments
// and sets the appropriate flags for the program
//
// int argc:            The number of command line arguements to parse through
// char **argv:         Char pointer holding all the arguments
// char **read_file:    Passed by refernce char pointer for infile name
// char **write_file:   PAssed by reference char pointer for outfile name
//
void get_options(int argc, char **argv, char **read_file, char **write_file) {
  int c = 0;
  // Loop until all command line arguements are read
  while ((c = getopt(argc, argv, OPTIONS)) != -1) {
    // Condition for the Stats flag
    if (c == 'v') {
      // Set the Stats bool to true
      Stats = true;
      // The infile flag
    } else if (c == 'i') {
      // Set the user infile flag to true
      user_infile = true;
      // Get the name of the text file and assign it to read_file
      (*read_file) = optarg;
      // The outfile flag
    } else if (c == 'o') {
      // Set the user outfile flag to true
      user_outfile = true;
      // Get the name of the text file and assign it to write_file
      (*write_file) = optarg;
    }
  }
}
