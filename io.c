#include "io.h"

// Buffer to hold symbols
static uint8_t buffer[BLOCK];
static uint32_t byte_count = 0;

// Counter to keep track of read symbols
static int rbytes = 0;

// Buffer and counter to hold bits
extern BitVector *bitbuf;
static uint32_t bit_index = 0;

//
// Wrapper for the read() syscall.
// Loops to read the specified number of bytes, or until input is exhausted.
// Returns the number of bytes read.
//
// infile:  File descriptor of the input file to read from.
// buf:     Buffer to store read bytes into.
// to_read: Number of bytes to read.
// returns: Number of bytes read.
//
int read_bytes(int infile, uint8_t *buf, int to_read) {
  // Counters to keep track of the total and read number of bytes
  int total_read = 0;
  int read_b = 0;
  // Loop to keep reading in bytes until a full block is read or until read()
  // returns 0
  while ((read_b = read(infile, buf + total_read, to_read - total_read)) > 0
         && total_read < to_read) {
    if (read_b < 0) {
      printf("Error: Failed to read infile!\n");
      exit(EXIT_FAILURE);
    }
    total_read += read_b;
  }
  return total_read;
}

//
// Wrapper for the write() syscall.
// Loops to write the specified number of bytes, or until nothing is written.
// Returns the number of bytes written.
//
// outfile:   File descriptor of the output file to write to.
// buf:       Buffer that stores the bytes to write out.
// to_write:  Number of bytes to write.
// returns:   Number of bytes written.
//
int write_bytes(int outfile, uint8_t *buf, int to_write) {
  // Counters to keep track of the total number of bytes written and currently
  // written
  int wbytes = 0;
  int total_written = 0;

  // Loop to keep calling write() until the specific block is written or until
  // there is nothing else to write
  do {
    wbytes = write(outfile, buf + total_written, to_write - total_written);
    if (wbytes < 0) {
      printf("Error: Failed to write to outfile!\n");
      exit(EXIT_FAILURE);
    }
    total_written += wbytes;
  } while (wbytes > 0 && total_written != to_write);
  return total_written;
}

//
// Reads in a FileHeader from the input file.
// Endianness of header fields are swapped if byte order isn't little endian.
//
// infile:  File descriptor of input file to read header from.
// header:  Pointer to memory where the bytes of the read header should go.
// returns: Void.
//
void read_header(int infile, FileHeader *header) {
  // Call the read() wrapper function casting the header struct to a uint8_T
  read_bytes(infile, (uint8_t *)header, sizeof(FileHeader));
  // increase the total bits read to the size of the FileHeader *8
  total_bits += sizeof(FileHeader) * 8;
  return;
}

//
// Writes a FileHeader to the output file.
// Endianness of header fields are swapped if byte order isn't little endian.
//
// outfile: File descriptor of output file to write header to.
// header:  Pointer to the header to write out.
// returns: Void.
//
void write_header(int outfile, FileHeader *header) {
  // Call the write() wrapper function casting the header struct to a uint8_t
  write_bytes(outfile, (uint8_t *)header, sizeof(FileHeader));
  // Increase the total bits written to the size of the FileHeader *8
  total_bits += (sizeof(FileHeader) * 8);
  return;
}

//
// "Reads" a symbol from the input file.
// The "read" symbol is placed into the pointer to sym (pass by reference).
// In reality, a block of symbols is read into a buffer.
// An index keeps track of the currently read symbol in the buffer.
// Once all symbols are processed, another block is read.
// If less than a block is read, the end of the buffer is updated.
// Returns true if there are symbols to be read, false otherwise.
//
// infile:  File descriptor of input file to read symbols from.
// sym:     Pointer to memory which stores the read symbol.
// returns: True if there are symbols to be read, false otherwise.
//
bool read_sym(int infile, uint8_t *byte) {
  // Condition to bytes from infile into the buffer
  if (byte_count == 0) {
    rbytes = read_bytes(infile, buffer, BLOCK);
    total_syms += rbytes;
    // Condition to return false is nothing else to read
    if (rbytes == 0) {
      return false;
    }
    // Assign byte to the symbol in each index in the buffer
    *byte = buffer[byte_count];
    byte_count++;
  } else {
    // Assign byte to the symbol in each index in the buffer
    *byte = buffer[byte_count];
    byte_count++;
    // Condition to reset the buffer counter when reached the end of buffer
    if (byte_count == rbytes) {
      byte_count = 0;
    }
  }
  return true;
}

//
// Buffers a pair. A pair is comprised of a symbol and an index.
// The bits of the symbol are buffered first, starting from the LSB.
// The bits of the index are buffered next, also starting from the LSB.
// bit_len bits of the index are buffered to provide a minimal representation.
// The buffer is written out whenever it is filled.
//
// outfile: File descriptor of the output file to write to.
// sym:     Symbol of the pair to buffer.
// index:   Index of the pair to buffer.
// bit_len: Number of bits of the index to buffer.
// returns: Void.
//
void buffer_pair(int outfile, uint16_t code, uint8_t sym, uint8_t bit_len) {
  // Loop to iterate bit_len times
  for (int bit = 0; bit < bit_len; bit++) {
    // Condition to check if bit counter reaches end of buffer
    // then write out buffer and reset bit counter
    if (bit_index == BLOCK * 8) {
      write_bytes(outfile, bitbuf->vector, BLOCK);
      bit_index = 0;
    }
    // Condition to check if bit at a specific position is on
    // then turn that bit on in the buffer
    // else turn it off
    if (code & (1 << bit)) {
      bv_set_bit(bitbuf, bit_index);
    } else {
      bv_clr_bit(bitbuf, bit_index);
    }
    bit_index++;
    total_bits++;
  }

  // Loop to iterate 8 times for a byte
  for (int bit = 0; bit < 8; bit++) {
    // Condition to check if bit counter reaches end of buffer
    // then write out buffer and reset bit counter
    if (bit_index == BLOCK * 8) {
      write_bytes(outfile, bitbuf->vector, BLOCK);
      bit_index = 0;
    }
    // Condition to check if bit at a specific position is on
    // then turn that bit on in the buffer
    // else turn it off
    if (sym & (1 << bit)) {
      bv_set_bit(bitbuf, bit_index);
    } else {
      bv_clr_bit(bitbuf, bit_index);
    }
    bit_index++;
    total_bits++;
  }
  return;
}

//
// Writes out any remaining pairs of symbols and indexes to the output file.
//
// outfile: File descriptor of the output file to write to.
// returns: Void.
//
void flush_pairs(int outfile) {
  // writes any remaining bytes left inside the buffer that is smaller than the block
  write_bytes(outfile, bitbuf->vector, (bit_index / 8) + 1);
  return;
}

//
// "Reads" a pair (symbol and index) from the input file.
// The "read" symbol is placed in the pointer to sym (pass by reference).
// The "read" index is placed in the pointer to index (pass by reference).
// In reality, a block of pairs is read into a buffer.
// An index keeps track of the current bit in the buffer.
// Once all bits have been processed, another block is read.
// The first 8 bits of the pair constitute the symbol, starting from the LSB.
// The next bit_len bits constitutes the index, starting from the the LSB.
// Returns true if there are pairs left to read in the buffer, else false.
// There are pairs left to read if the read index is not STOP_INDEX.
//
// infile:  File descriptor of the input file to read from.
// sym:     Pointer to memory which stores the read symbol.
// index:   Pointer to memory which stores the read index.
// bit_len: Length in bits of the index to read.
// returns: True if there are pairs left to read, false otherwise.
//
bool read_pair(int infile, uint16_t *code, uint8_t *sym, uint8_t bit_len) {
  (*code) = 0;
  (*sym) = 0;
  // Loop to iterate bit_len times
  for (int bit = 0; bit < bit_len; bit++) {
    // Condition to check if the bit counter is at the end of the buffer or
    // the buffer has nothin in it, then read a new block and reset the bit
    // counter
    if (bit_index == BLOCK * 8 || bit_index == 0) {
      rbytes = read_bytes(infile, bitbuf->vector, BLOCK);
      bit_index = 0;
      // Condition to check if the amount of bytes read is 0, if so then
      // return false
      if (rbytes == 0) {
        return false;
      }
    }

    // Condition to check if the bit within the bit buffer at a specifc position
    // is on, if so add that bit value to the code
    if (bv_get_bit(bitbuf, bit_index)) {
      (*code) += 1 << bit;
    }
    bit_index++;
    total_bits++;
  }

  // Loop to iterate 8 times for a byte
  for (int bit = 0; bit < 8; bit++) {
    // Condition to check if the bit counter is at the end of the buffer or
    // the buffer has nothin in it, then read a new block and reset the bit
    // counter
    if (bit_index == BLOCK * 8 || bit_index == 0) {
      rbytes = read_bytes(infile, bitbuf->vector, BLOCK);
      bit_index = 0;
      // Condition to check if the amount of bytes read is 0, if so then
      // return false
      if (rbytes == 0) {
        return false;
      }
    }
    // Condition to check if the bit within the bit buffer at a specifc position
    // is on, if so add that bit value to the code
    if (bv_get_bit(bitbuf, bit_index)) {
      (*sym) += 1 << bit;
    }
    bit_index++;
    total_bits++;
  }
  //Condition if the code received is the STOP_CODE, then return false
  if ((*code) == STOP_CODE) {
    return false;
  }
  return true;
}

//
// Buffers a Word, or more specifically, the symbols of a Word.
// Each symbol of the Word is placed into a buffer.
// The buffer is written out when it is filled.
//
// outfile: File descriptor of the output file to write to.
// w:       Word to buffer.
// returns: Void.
//
void buffer_word(int outfile, Word *w) {
  // Loop through all the elements in the words symbols array
  for (int i = 0; i < w->len; i++) {
    // Add every symbol in the word to the buffer
    buffer[byte_count] = w->syms[i];
    byte_count++;
    total_syms++;
    // Condition to check if the byte counter is at the end of the buffer
    // if so then write out the buffer to the outfile and reset the byte counter
    if (byte_count == BLOCK) {
      write_bytes(outfile, buffer, BLOCK);
      byte_count = 0;
    }
  }
  return;
}

//
// Writes out any remaining symbols in the buffer.
//
// outfile: File descriptor of the output file to write to.
// returns: Void.
//
void flush_words(int outfile) {
  // Writes out any remainder bytes smaller than the block thats still in the buffer
  write_bytes(outfile, buffer, byte_count);
  return;
}
