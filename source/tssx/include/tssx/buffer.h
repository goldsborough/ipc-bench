#ifndef BUFFER_H
#define BUFFER_H

#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "tssx/timeouts.h"

/******************** DEFINITIONS ********************/

#define BUFFER_ERROR -1
#define BUFFER_SUCCESS 0

/******************** STRUCTURES ********************/

typedef enum { READ, WRITE } Operation;

typedef struct Buffer {
	// The current size of the buffer (number of bytes used)
	atomic_int size;

	// The length of the memory segment in bytes
	size_t capacity;

	// Collection of timeout values
	Timeouts timeouts;

	// The read index
	size_t read;

	// The write index
	size_t write;

} Buffer;

/******************** INTERFACE ********************/

Buffer* create_buffer(void* shared_memory,
											size_t requested_capacity,
											const Timeouts* timeouts);

size_t buffer_write(Buffer* buffer, void* data, size_t data_size);
size_t buffer_read(Buffer* buffer, void* data, size_t data_size);

size_t buffer_peek(Buffer* buffer, void* data, size_t data_size);
size_t buffer_skip(Buffer* buffer, size_t number_of_bytes);

void buffer_clear(Buffer* buffer);

bool buffer_is_full(Buffer* buffer);
bool buffer_is_empty(Buffer* buffer);

#ifdef TSSX_SUPPORT_BUFFER_TIMEOUTS
void buffer_set_timeout(Buffer* buffer,
												Operation operation,
												cycle_t new_timeout);
bool buffer_has_timeout(Buffer* buffer, Operation operation);
#endif

size_t buffer_free_space(Buffer* buffer);

/******************** PRIVATE ********************/

typedef bool (*Condition)(Buffer*, size_t);

void* _start_pointer(Buffer* buffer);
void* _end_pointer(Buffer* buffer);
void* _read_pointer(Buffer* buffer);
void* _write_pointer(Buffer* buffer);

void* _pointer_to(Buffer* buffer, size_t index);
ptrdiff_t _index_at(Buffer* buffer, void* pointer);

void _wrap_read(Buffer* buffer, void** data, size_t* data_size, size_t delta);
void _wrap_write(Buffer* buffer, void** data, size_t* data_size, size_t delta);
void _reduce_data(void** data, size_t* data_size, size_t delta);

bool _timeout_elapsed(Buffer* buffer, cycle_t elapsed, Operation operation);
bool _level_elapsed(Buffer* buffer, size_t level, cycle_t elapsed);

void _pause();
cycle_t _now();

int _escalation_level(Buffer* buffer, cycle_t start_time, Operation operation);

bool _ready_for(Buffer* buffer, Operation operation, size_t requested_size);
int _block(Buffer* buffer, size_t requested_size, Operation operation);

#endif /* BUFFER_H */
