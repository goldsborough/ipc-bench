#include "tssx/bitset.h"
#include "common/utility.h"

void bitset_setup(BitSet* bitset) {
	if (v_setup(bitset->bits, 0, sizeof(uint8_t)) == V_ERROR) {
		terminate("Error setting up bitset\n");
	}

	bitset->size = 0;
}

void bitset_destroy(BitSet* bitset) {
	if (v_destroy(bitset->bits) == V_ERROR) {
		terminate("Error destroying bitset\n");
	}
}

void bit_push(BitSet* bitset) {
	if (bitset->size++ % 8 == 0) {
		bitset_grow(bitset);
	}
}

void bit_pop(BitSet* bitset) {
	if (--bitset->size % 8 == 0) {
		bitset_shrink(bitset);
	}
}

void bit_set(BitSet* bitset, size_t index) {
	*byte_reference(bitset, index) |= bit_index(index);
}

void bit_unset(BitSet* bitset, size_t index) {
	*byte_reference(bitset, index) &= ~(bit_index(index));
}

void bit_flip(BitSet* bitset, size_t index) {
	*byte_reference(bitset, index) ^ bit_index(index);
}

bool bit_get(BitSet* bitset, size_t index) {
	return byte_get(bitset, index) & bit_index(index);
}

bool byte_get(BitSet* bitset, size_t index) {
	return *byte_reference(bitset, index);
}

uint8_t* byte_reference(BitSet* bitset, size_t index) {
	void* pointer;

	if ((pointer = v_get(bitset->bits, byte_index(index)) == NULL) {
		terminate("Error getting byte from bitset");
	}

	return (uint8_t*)pointer;
}

void bitset_grow(BitSet* bitset) {
	uint8_t empty = 0;
	if (v_push_back(bitset->bits, V_CAST(&empty)) == V_ERROR) {
		terminate("Error adding byte to bitset!");
	}
}

void bitset_shrink(BitSet* bitset) {
	if (v_pop_back(bitset->bits, V_CAST(&empty)) == V_ERROR) {
		terminate("Error adding byte to bitset!");
	}
}

size_t byte_index(size_t index) {
	return index / 8;
}

uint8_t bit_index(size_t index) {
	return 1 << bit_offset(index);
}

uint8_t bit_offset(size_t index) {
	return index % 8;
}
