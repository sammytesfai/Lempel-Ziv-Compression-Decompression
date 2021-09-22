#include "bv.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// This function takes a integer as an argument and creates and initializes
// a new structure for the Bitvector.
// uint32_t bit_len - This is the integer used to identify how many indexes
// to allocate to get enough bits for every number up until the length passed.*/
BitVector *bv_create(uint32_t bit_len) {
  BitVector *new_vector = (BitVector *)calloc(1, sizeof(BitVector));
  // Error check to see if memory successfully allocated
  if (new_vector == NULL) {
    printf("Error: Failed to allocate memory for stack member!");
    exit(EXIT_FAILURE);
  }
  new_vector->length = bit_len;
  new_vector->vector = (uint8_t *)calloc((bit_len / 8) + 1, sizeof(uint8_t));
  // Error check to see if memory was successfully allocated
  if (new_vector->vector == NULL) {
    printf("Error: Failed to allocate memory for bit vector!");
    exit(EXIT_FAILURE);
  }
  return new_vector;
}

// This function takes free any allocated memory by the bit vector struct
// BitVector *v - is the struct pointer to be to the BitVector member
void bv_delete(BitVector *v) {
  free(v->vector);
  free(v);
  return;
}

// Function returns the length in bit used in in the BitVector
uint32_t bv_get_len(BitVector *v) {
  return v->length;
}

// Function turns the bit on that correlates to the interger being passed
// BitVector *v - is the struct pointer to be to the BitVector member
// uint32_t i - is the integer that correlates to the bit in the BitVector
void bv_set_bit(BitVector *v, uint32_t i) {
  // Condition to check if the bit to set is not already set on
  if (!bv_get_bit(v, i)) {
    // Identify the index of bit by dividing by 8
    uint32_t index = i / 8;
    // Identifying bit number within index
    uint32_t bit = i % 8;
    // Turns bit on using OR and 1 shifted to the left bit number of times
    v->vector[index] |= 1 << bit;
  }
  return;
}

// Function turns the bit off that correlates to the integer being passed
// BitVector *v - is the struct pointer to be to the BitVector member
// uint32_t i - is the integer that correlates to the bit in the BitVector
void bv_clr_bit(BitVector *v, uint32_t i) {
  // Condition to check if the bit to be turned of is already off
  if (bv_get_bit(v, i)) {
    // Identify the index of bit by dividing by 8
    uint32_t index = i / 8;
    // Identifying bit number within index
    uint32_t bit = i % 8;
    v->vector[index] ^= 1 << bit;
  }
  return;
}

// This function returns the bit value at a specifc position
// BitVector *v - is the struct pointer to be to the BitVector member
// uint32_t i - is the integer that correlates to the bit in the BitVector
uint8_t bv_get_bit(BitVector *v, uint32_t i) {
  // Identify the index of bit by dividing by 8
  uint32_t index = i / 8;
  // Identifying bit number within index
  uint8_t bit = i % 8;
  return ((v->vector[index] & 1 << bit)) == 1 << bit;
}

// Function sets all the bit in the bit vector to 1(on)
// BitVector *v - is the struct pointer to be to the BitVector member
void bv_set_all_bits(BitVector *v) {
  for (uint32_t i = 0; i <= v->length / 8; i++) {
    v->vector[i] = 0xFF;
  }
  return;
}
