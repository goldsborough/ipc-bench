#include <assert.h>
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


#include <stdio.h>


#include "common/utility.h"
#include "tssx/buffer.h"

Buffer*
create_buffer(void* shared_memory, int requested_capacity, double timeout) {
	Buffer* buffer = (Buffer*)shared_memory;

	buffer->capacity = requested_capacity;
	buffer->timeout = timeout;

	buffer_clear(buffer);

	return buffer;
}

int buffer_write(Buffer* buffer, void* data, int data_size) {
	int right_space = 0;

	if (buffer == NULL) return ERROR;
	if (data == NULL) return ERROR;
	if (data_size == 0) return 0;
	if (_block(buffer, data_size, _writeable) == TIMEOUT) return ERROR;
	//while (data_size > buffer_free_space(buffer)) nsleep(1);

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
	buffer->size += data_size;

		// How many bytes we wrote
	return data_size + right_space;
}

int buffer_read(Buffer* buffer, void* data, int data_size) {
	int right_space = 0;

	if (buffer == NULL) return ERROR;
	if (data == NULL) return ERROR;
	if (data_size == 0) return 0;
	if (_block(buffer, data_size, _readable) == TIMEOUT) return ERROR;
	//while (data_size > buffer->size) nsleep(1);

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
	buffer->size -= data_size;

	// How many bytes we wrote
	return data_size + right_space;
}

int buffer_peek(Buffer* buffer, void* data, int data_size) {
	int return_value;
	int old_size;
	int old_read;

	old_size = buffer->size;
	old_read = buffer->read;

	return_value = buffer_read(buffer, data, data_size);

	// Restore
	buffer->size = old_size;
	buffer->read = old_read;

	return return_value;
}

int buffer_skip(Buffer* buffer, int how_many) {
	assert(buffer != NULL);

	if (how_many < 0) return ERROR;
	if (how_many > buffer->size) return ERROR;

	buffer->read = (buffer->read + how_many) % buffer->capacity;

	return how_many;
}

void buffer_clear(Buffer* buffer) {
	buffer->read = 0;
	buffer->write = 0;
	buffer->size = 0;
}

int buffer_is_full(Buffer* buffer) {
	return buffer->size == buffer->capacity;
}

int buffer_is_empty(Buffer* buffer) {
	return buffer->size == 0;
}

int buffer_has_timeout(Buffer* buffer) {
	return buffer->timeout != 0;
}

int buffer_free_space(Buffer* buffer) {
	return buffer->capacity - buffer->size;
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

void* _pointer_to(Buffer* buffer, int index) {
	assert(index >= 0);
	assert(index <= buffer->capacity);

	return _start_pointer(buffer) + index;
}
int _index_at(Buffer* buffer, void* pointer) {
	assert(pointer >= _start_pointer(buffer));
	assert(pointer <= _end_pointer(buffer));

	return pointer - _start_pointer(buffer);
}

void _wrap_read(Buffer* buffer, void** data, int* data_size, int delta) {
	buffer->read = 0;
	buffer->size -= delta;
	_reduce_data(data, data_size, delta);
}

void _wrap_write(Buffer* buffer, void** data, int* data_size, int delta) {
	buffer->write = 0;
	buffer->size += delta;
	_reduce_data(data, data_size, delta);
}

void _reduce_data(void** data, int* data_size, int delta) {
	*data_size -= delta;
	*data += delta;
}

void _check_write_error(int return_code) {
	if (return_code != 0) {
		throw("Error writing to buffer");
	}
}

void _check_read_error(int return_code) {
	if (return_code != 0) {
		throw("Error reading from buffer");
	}
}

int _writeable(Buffer* buffer, int requested_size) {
	return requested_size <= buffer_free_space(buffer);
}

int _readable(Buffer* buffer, int requested_size) {
	return requested_size <= buffer->size;
}

double _now() {
	return ((double)clock()) / CLOCKS_PER_SEC;
}

int _escalation_level(Buffer* buffer, double start_time) {
	double elapsed = _now() - start_time;

	if (buffer_has_timeout(buffer) && elapsed > buffer->timeout) {
		errno = EWOULDBLOCK;
		return TIMEOUT;
	} else if (elapsed > LEVEL_TWO_TIME) {
		return LEVEL_THREE;
	} else if (elapsed > LEVEL_ONE_TIME) {
		return LEVEL_TWO;
	} else {
		return LEVEL_ONE;
	}
}

int _block(Buffer* buffer, int requested_size, Condition condition) {
	double start_time = _now();

	while (!condition(buffer, requested_size)) {
		// Busy waiting sucks really, really, really hard!
		// sched_yield is really, really, really good
		switch (_escalation_level(buffer, start_time)) {
		  case LEVEL_ONE:
		  case LEVEL_TWO: sched_yield(); break;
		  case LEVEL_THREE: usleep(1); break;
		  case TIMEOUT: return -1;
		}
	}

	start_time *= 1e6;
	//double end_time = _now() * 1e6;

	//printf("Blocked: %f - %f: %f\n", start_time, end_time, end_time - start_time);

	return 0;
}
