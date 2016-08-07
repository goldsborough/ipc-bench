#define _XOPEN_SOURCE 500
#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <x86intrin.h>
#include <xmmintrin.h>

#ifdef DEBUG
#include <stdio.h>
#endif

#include "common/utility.h"
#include "tssx/buffer.h"
#include "tssx/definitions.h"
#include "tssx/timeouts.h"

Buffer *create_buffer(void *shared_memory,
											size_t requested_capacity,
											const Timeouts *timeouts) {
	Buffer *buffer = (Buffer *)shared_memory;

	buffer->capacity = requested_capacity;
	buffer->timeouts = *timeouts;
	buffer_clear(buffer);

	return buffer;
}

size_t buffer_write(Buffer *buffer, const void *data, size_t bytes_to_write) {
	size_t right_space = 0;

	if (buffer == NULL) return ERROR;
	if (data == NULL) return ERROR;
	if (bytes_to_write == 0) return 0;

	// Block or return, depending on blocking configuration for the socket
	// After the branch there is enough space to do the operation
	size_t available_space = _determine_available_space(buffer, WRITE);
	bytes_to_write =
			available_space < bytes_to_write ? available_space : bytes_to_write;

	// The == is when the buffer is empty
	if (buffer->write >= buffer->read) {
		// Available space to the right of the write pointer
		right_space = buffer->capacity - buffer->write;

		if (bytes_to_write >= right_space) {
			// Write first portion, up to the end of the buffer
			memcpy(_write_pointer(buffer), data, right_space);
			_wrap_write(buffer, &data, &bytes_to_write, right_space);
		} else {
			right_space = 0;
		}
	}

	memcpy(_write_pointer(buffer), data, bytes_to_write);

	buffer->write += bytes_to_write;
	atomic_fetch_add(&buffer->size, bytes_to_write);

	// How many bytes we wrote
	return bytes_to_write + right_space;
}

size_t buffer_read(Buffer *buffer, void *data, size_t bytes_to_read) {
	size_t right_space = 0;

	if (buffer == NULL) return ERROR;
	if (data == NULL) return ERROR;
	if (bytes_to_read == 0) return 0;

	// Block or return, depending on blocking configuration for the socket
	// After the branch there is enough space to do the operation
	size_t available_space = _determine_available_space(buffer, READ);
	bytes_to_read =
			available_space < bytes_to_read ? available_space : bytes_to_read;

	// Read bytes_to_read from buffer
	if (buffer->read >= buffer->write) {
		right_space = buffer->capacity - buffer->read;

		if (bytes_to_read >= right_space) {
			// Read first portion, then wrap around and write the rest below
			memcpy(data, _read_pointer(buffer), right_space);
			_wrap_read(buffer, &data, &bytes_to_read, right_space);
		} else {
			right_space = 0;
		}
	}

	memcpy(data, _read_pointer(buffer), bytes_to_read);

	buffer->read += bytes_to_read;
	atomic_fetch_sub(&buffer->size, bytes_to_read);

	// How many bytes we read (Needs to be -1 if nothing was read)
	return (bytes_to_read + right_space) == 0 ? (size_t)-1
																						: (bytes_to_read + right_space);
}

size_t buffer_peek(Buffer *buffer, void *data, size_t data_size) {
	size_t return_value;
	size_t old_size;
	size_t old_read;

	old_size = atomic_load(&buffer->size);
	old_read = buffer->read;

	return_value = buffer_read(buffer, data, data_size);

	// Restore
	atomic_store(&buffer->size, old_size);
	buffer->read = old_read;

	return return_value;
}

size_t buffer_skip(Buffer *buffer, size_t number_of_bytes) {
	assert(buffer != NULL);

	if (number_of_bytes > atomic_load(&buffer->size)) return ERROR;

	buffer->read = (buffer->read + number_of_bytes) % buffer->capacity;

	return number_of_bytes;
}

void buffer_clear(Buffer *buffer) {
	buffer->read = 0;
	buffer->write = 0;
	atomic_store(&buffer->size, 0);
}

bool buffer_is_full(Buffer *buffer) {
	return atomic_load(&buffer->size) == buffer->capacity;
}

bool buffer_is_empty(Buffer *buffer) {
	return atomic_load(&buffer->size) == 0;
}

bool buffer_ready_for(Buffer *buffer, Operation operation) {
	if (operation == READ) {
		return !buffer_is_empty(buffer);
	} else {
		return !buffer_is_full(buffer);
	}
}

#ifdef TSSX_SUPPORT_BUFFER_TIMEOUTS
void buffer_set_timeout(Buffer *buffer,
												Operation operation,
												cycle_t new_timeout) {
	buffer->timeouts.timeout[operation] = new_timeout;
}

bool buffer_has_timeout(Buffer *buffer, Operation operation) {
	return buffer->timeouts.timeout[operation] != 0;
}
#endif

size_t buffer_free_space(Buffer *buffer) {
	return buffer->capacity - atomic_load(&buffer->size);
}

/******* PRIVATE *******/

void *_start_pointer(Buffer *buffer) {
	return (void *)++buffer;
}

void *_end_pointer(Buffer *buffer) {
	return _pointer_to(buffer, buffer->capacity);
}

void *_read_pointer(Buffer *buffer) {
	return _pointer_to(buffer, buffer->read);
}

void *_write_pointer(Buffer *buffer) {
	return _pointer_to(buffer, buffer->write);
}

void *_pointer_to(Buffer *buffer, size_t index) {
	assert(index <= buffer->capacity);

	return _start_pointer(buffer) + index;
}

ptrdiff_t _index_at(Buffer *buffer, void *pointer) {
	assert(pointer >= _start_pointer(buffer));
	assert(pointer <= _end_pointer(buffer));

	return pointer - _start_pointer(buffer);
}

void _wrap_read(Buffer *buffer, void **data, size_t *data_size, size_t delta) {
	buffer->read = 0;
	atomic_fetch_sub(&buffer->size, delta);
	_reduce_data((const void **)data,
							 data_size,
							 delta);// TODO: Is this cast ok ???
}

void _wrap_write(Buffer *buffer,
								 const void **data,
								 size_t *data_size,
								 size_t delta) {
	buffer->write = 0;
	atomic_fetch_add(&buffer->size, delta);
	_reduce_data(data, data_size, delta);
}

void _reduce_data(const void **data, size_t *data_size, size_t delta) {
	*data_size -= delta;
	*data += delta;
}

#ifdef TSSX_SUPPORT_BUFFER_TIMEOUTS
bool _timeout_elapsed(Buffer *buffer, cycle_t elapsed, Operation operation) {
	if (!buffer_has_timeout(buffer, operation)) return false;
	return elapsed > buffer->timeouts.timeout[operation];
}
#endif

bool _level_elapsed(Buffer *buffer, size_t level, cycle_t elapsed) {
	return elapsed > buffer->timeouts.levels[level];
}

void _pause() {
	_mm_pause();
}

cycle_t _now() {
	return __rdtsc();
}

int _escalation_level(Buffer *buffer, cycle_t start_time, Operation operation) {
	cycle_t elapsed = _now() - start_time;

#ifdef TSSX_SUPPORT_BUFFER_TIMEOUTS
	if (_timeout_elapsed(buffer, elapsed, operation)) {
		errno = EWOULDBLOCK;
		return TIMEOUT;
	}
#endif

	if (!_level_elapsed(buffer, LEVEL_ZERO, elapsed)) {
		return LEVEL_ZERO;
	} else if (!_level_elapsed(buffer, LEVEL_ONE, elapsed)) {
		return LEVEL_ONE;
	} else {
		return LEVEL_TWO;
	}
}

size_t _determine_available_space(Buffer *buffer, Operation operation) {
	size_t available_space;
	if (buffer->timeouts.non_blocking[operation]) {
		available_space = _get_available_space(buffer, operation);
	} else {
		available_space = _block_for_available_space(buffer, operation);
	}
	if (available_space == 0) {
		return 0;
	}

	return available_space;
}

size_t _get_available_space(Buffer *buffer, Operation operation) {
	if (operation == READ) {
		return atomic_load(&buffer->size);
	} else {
		return buffer_free_space(buffer);
	}
}

size_t _block_for_available_space(Buffer *buffer, Operation operation) {
	cycle_t start_time;

	start_time = _now();
	size_t space = _get_available_space(buffer, operation);
	while (space == 0) {
		switch (_escalation_level(buffer, start_time, operation)) {
			case LEVEL_ZERO: _pause(); break;
			case LEVEL_ONE: sched_yield(); break;
			case LEVEL_TWO: usleep(1); break;
#ifdef TSSX_SUPPORT_BUFFER_TIMEOUTS
			case TIMEOUT: print_error("not impl"); return TIMEOUT;
#endif
		}
		space = _get_available_space(buffer, operation);
	}

	return space;
}
