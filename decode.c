#include "bv.h"
#include "code.h"
#include "io.h"
#include "word.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

// Defined option for the command line arguements
#define OPTIONS "vi:o:"

// Global variables to count bytes
// // for compression and decompression
uint64_t total_syms = 0;
uint64_t total_bits = 0;

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
// uint16_t nex_code:           16 bit code integer
//
uint8_t bit_length(uint16_t next_code);

//
// This function parses through the command line arguments
// and sets the appropriate flags for the program
//
// int argc:            The number of command line arguements to parse through
// char **argv:         Char pointer holding all the arguments
// char **read_file:    Passed by refernce char pointer for infile name
// char **write_file:   PAssed by reference char pointer for outfile name
//
void get_options(int argc, char **argv, char **read_file, char **write_file);

int main(int argc, char **argv) {
  // Initialize char pointers for files names
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

  // Read header file from infile and copy protection number
  // to outfile
  read_header(infile, header);
  fchmod(outfile, header->protection);

  WordTable *wt;

  // Check if the magic number read in from the file is the same
  // as the MAGIC number macro else exit
  if (header->magic == MAGIC) {
    // Create Read buffer with 4096 elements
    bitbuf = bv_create(BLOCK * 8);

    wt = wt_create();
    uint8_t curr_sym = 0;
    uint16_t curr_code = 0;
    uint16_t next_code = START_CODE;

    // Loop until there are no more bits to procress in the read buffer
    while (read_pair(infile, &curr_code, &curr_sym, bit_length(next_code))) {
      // Puts a new word or an appended word into the wordtable
      wt[next_code] = word_append_sym(wt[curr_code], curr_sym);
      // Buffer the word into the symbol buffer
      buffer_word(outfile, wt[next_code]);
      next_code++;
      // If code reaches its max value reset the word table and next_code
      if (next_code == MAX_CODE) {
        wt_reset(wt);
        next_code = START_CODE;
      }
    }
    // Flush any remaining symbols from the buffer into the oufile
    flush_words(outfile);
  } else {
    printf("The encoded file can not be decoded with this program!\n");
    free(header);
    exit(EXIT_FAILURE);
  }

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

  // Deallocate memory from Word ADT, bit buffer, and File Header
  bv_delete(bitbuf);
  wt_delete(wt);
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
