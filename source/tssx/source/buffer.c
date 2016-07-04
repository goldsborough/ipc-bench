#define _XOPEN_SOURCE 500
#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <x86intrin.h>
#include <xmmintrin.h>

#include "common/utility.h"
#include "tssx/buffer.h"
#include "tssx/timeouts.h"

Buffer* create_buffer(void* shared_memory,
											size_t requested_capacity,
											const Timeouts* timeouts) {
	Buffer* buffer = (Buffer*)shared_memory;

	buffer->capacity = requested_capacity;
	buffer->timeouts = *timeouts;
	buffer_clear(buffer);

	return buffer;
}

size_t buffer_write(Buffer* buffer, void* data, size_t data_size) {
	size_t right_space = 0;

	if (buffer == NULL) return BUFFER_ERROR;
	if (data == NULL) return BUFFER_ERROR;
	if (data_size == 0) return 0;

	if (_block(buffer, data_size, WRITE) == TIMEOUT) return BUFFER_ERROR;

	// The == is when the buffer is empty
	if (buffer->write >= buffer->read) {
		// Available space to the right of the write pointer
		right_space = buffer->capacity - buffer->write;

		if (data_size >= right_space) {
			// Write first portion, up to the end of the buffer
			memcpy(_write_pointer(buffer), data, right_space);
			_wrap_write(buffer, &data, &data_size, right_space);
		}
	}

	memcpy(_write_pointer(buffer), data, data_size);

	buffer->write += data_size;
	atomic_fetch_add(&buffer->size, data_size);

	// How many bytes we wrote
	return data_size + right_space;
}

size_t buffer_read(Buffer* buffer, void* data, size_t data_size) {
	size_t right_space = 0;

	if (buffer == NULL) return BUFFER_ERROR;
	if (data == NULL) return BUFFER_ERROR;
	if (data_size == 0) return 0;

	if (_block(buffer, data_size, READ) == TIMEOUT) return BUFFER_ERROR;

	if (buffer->read >= buffer->write) {
		right_space = buffer->capacity - buffer->read;

		if (data_size >= right_space) {
			// Read first portion, then wrap around and write the rest below
			memcpy(data, _read_pointer(buffer), right_space);
			_wrap_read(buffer, &data, &data_size, right_space);
		}
	}

	memcpy(data, _read_pointer(buffer), data_size);

	buffer->read += data_size;
	atomic_fetch_sub(&buffer->size, data_size);

	// How many bytes we wrote
	return data_size + right_space;
}

size_t buffer_peek(Buffer* buffer, void* data, size_t data_size) {
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

size_t buffer_skip(Buffer* buffer, size_t number_of_bytes) {
	assert(buffer != NULL);

	if (number_of_bytes > atomic_load(&buffer->size)) return BUFFER_ERROR;

	buffer->read = (buffer->read + number_of_bytes) % buffer->capacity;

	return number_of_bytes;
}

void buffer_clear(Buffer* buffer) {
	buffer->read = 0;
	buffer->write = 0;
	atomic_store(&buffer->size, 0);
}

bool buffer_is_full(Buffer* buffer) {
	return atomic_load(&buffer->size) == buffer->capacity;
}

bool buffer_is_empty(Buffer* buffer) {
	return atomic_load(&buffer->size) == 0;
}

#ifdef TSSX_SUPPORT_BUFFER_TIMEOUTS
void buffer_set_timeout(Buffer* buffer,
												Operation operation,
												cycle_t new_timeout) {
	if (operation == READ) {
		buffer->timeouts.read_timeout = new_timeout;
	} else {
		buffer->timeouts.write_timeout = new_timeout;
	}
}

bool buffer_has_timeout(Buffer* buffer, Operation operation) {
	if (operation == READ) {
		return buffer->timeouts.read_timeout != 0;
	} else {
		return buffer->timeouts.write_timeout != 0;
	}
}
#endif /* TSSX_SUPPORT_BUFFER_TIMEOUTS */

size_t buffer_free_space(Buffer* buffer) {
	return buffer->capacity - atomic_load(&buffer->size);
}

/******* PRIVATE *******/

void* _start_pointer(Buffer* buffer) {
	return (void*)++buffer;
}

void* _end_pointer(Buffer* buffer) {
	return _pointer_to(buffer, buffer->capacity);
}

void* _read_pointer(Buffer* buffer) {
	return _pointer_to(buffer, buffer->read);
}

void* _write_pointer(Buffer* buffer) {
	return _pointer_to(buffer, buffer->write);
}

void* _pointer_to(Buffer* buffer, size_t index) {
	assert(index <= buffer->capacity);

	return _start_pointer(buffer) + index;
}

ptrdiff_t _index_at(Buffer* buffer, void* pointer) {
	assert(pointer >= _start_pointer(buffer));
	assert(pointer <= _end_pointer(buffer));

	return pointer - _start_pointer(buffer);
}

void _wrap_read(Buffer* buffer, void** data, size_t* data_size, size_t delta) {
	buffer->read = 0;
	atomic_fetch_sub(&buffer->size, delta);
	_reduce_data(data, data_size, delta);
}

void _wrap_write(Buffer* buffer, void** data, size_t* data_size, size_t delta) {
	buffer->write = 0;
	atomic_fetch_add(&buffer->size, delta);
	_reduce_data(data, data_size, delta);
}

void _reduce_data(void** data, size_t* data_size, size_t delta) {
	*data_size -= delta;
	*data += delta;
}

#ifdef TSSX_SUPPORT_BUFFER_TIMEOUTS
bool _timeout_elapsed(Buffer* buffer, cycle_t elapsed, Operation operation) {
	if (!buffer_has_timeout(buffer, operation)) return false;
	if (operation == READ) {
		return elapsed > buffer->timeouts.read_timeout;
	} else {
		return elapsed > buffer->timeouts.write_timeout;
	}
}
#endif /* TSSX_SUPPORT_BUFFER_TIMEOUTS */

bool _level_elapsed(Buffer* buffer, size_t level, cycle_t elapsed) {
	return elapsed > buffer->timeouts.levels[level];
}

void _pause() {
	_mm_pause();
}

cycle_t _now() {
	return __rdtsc();
}

int _escalation_level(Buffer* buffer, cycle_t start_time, Operation operation) {
	cycle_t elapsed = _now() - start_time;

	// #ifdef TSSX_SUPPORT_BUFFER_TIMEOUTS
	// 	if (_timeout_elapsed(buffer, elapsed, operation)) {
	// 		errno = EWOULDBLOCK;
	// 		return TIMEOUT;
	// 	}
	// #endif /* TSSX_SUPPORT_BUFFER_TIMEOUTS */

	if (!_level_elapsed(buffer, LEVEL_ZERO, elapsed)) {
		return LEVEL_ZERO;
	} else if (!_level_elapsed(buffer, LEVEL_ONE, elapsed)) {
		return LEVEL_ONE;
	} else {
		return LEVEL_TWO;
	}
}

bool _ready_for(Buffer* buffer, Operation operation, size_t requested_size) {
	if (operation == READ) {
		return requested_size <= atomic_load(&buffer->size);
	} else {
		return requested_size <= buffer_free_space(buffer);
	}
}

int _block(Buffer* buffer, size_t requested_size, Operation operation) {
	cycle_t start_time = _now();

	while (!_ready_for(buffer, operation, requested_size)) {
		switch (_escalation_level(buffer, start_time, operation)) {
			case LEVEL_ZERO: _pause(); break;
			case LEVEL_ONE: sched_yield(); break;
			case LEVEL_TWO: usleep(1); break;
#ifdef TSSX_SUPPORT_BUFFER_TIMEOUTS
			case TIMEOUT: return TIMEOUT;
#endif /* TSSX_SUPPORT_BUFFER_TIMEOUTS */
		}
	}

	return BUFFER_SUCCESS;
}
