#ifndef BITSET_H
#define BITSET_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "tssx/vector.h"

typedef struct BitSet {
	struct Vector bits;
	size_t size;
#define capacity bits.size
} BitSet;

void bitset_setup(BitSet* bitset);
void bitset_destroy(BitSet* bitset);

void bit_push(BitSet* bitset, bool value);
void bit_push_one(BitSet* bitset);
void bit_push_zero(BitSet* bitset);
void bit_pop(BitSet* bitset);

void bit_set(BitSet* bitset, size_t index);
void bit_unset(BitSet* bitset, size_t index);
void bit_assign(BitSet* bitset, size_t index, bool value);
void bit_flip(BitSet* bitset, size_t index);

bool bit_get(BitSet* bitset, size_t index);

uint8_t byte_get(BitSet* bitset, size_t index);
uint8_t* byte_reference(BitSet* bitset, size_t index);

void bitset_grow(BitSet* bitset);
void bitset_shrink(BitSet* bitset);

size_t byte_index(size_t index);
uint8_t bit_index(size_t index);
uint8_t bit_offset(size_t index);

#endif /* BITSET_H */
