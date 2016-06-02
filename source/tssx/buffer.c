#include <assert.h>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "common/utility.h"
#include "tssx/buffer.h"

Buffer*
create_buffer(void* shared_memory, int requested_capacity, double timeout) {
	Buffer* buffer = (Buffer*)shared_memory;

	buffer->memory = buffer + sizeof(Buffer);
	buffer->capacity = requested_capacity;
	buffer->timeout = timeout;

	buffer_clear(buffer);

	return buffer;
}

int buffer_write(Buffer* buffer, void* data, int data_size) {
	int space;

	assert(buffer != NULL);
	assert(data != NULL);

	if (data_size == 0) return 0;
	if (_block(buffer, data_size, _enough_space) == TIMEOUT) return TIMEOUT;

	// The == is when the buffer is empty
	if (buffer->write >= buffer->read) {
		// Available space to the right of the write pointer
		space = buffer_end(buffer) - buffer->write;

		// Enough forward space
		if (data_size <= space) {
			memcpy(buffer->write, data, data_size);

		} else {
			// Write first portion, up to the end of the buffer
			memcpy(buffer->write, data, space);

			buffer->write = buffer->memory;
			data_size -= space;
			data += space;

			// Write second portion
			memcpy(buffer->write, data, data_size);
		}
	} else {
		memcpy(buffer->write, data, data_size);
	}

	buffer->write += data_size;
	buffer->size += data_size;

	return data_size;
}

int buffer_read(Buffer* buffer, void* data, int data_size) {
	int space;

	assert(buffer != NULL);
	assert(data != NULL);

	if (data_size <= 0) return 0;
	if (_block(buffer, data_size, _enough_data) == TIMEOUT) return TIMEOUT;

	if (buffer->read >= buffer->write) {
		// Available space to the right of the write pointer
		space = buffer_end(buffer) - buffer->write;

		if (data_size <= space) {
			memcpy(data, buffer->read, data_size);
		} else {
			// Read first portion, up to the end of the buffer
			memcpy(data, buffer->read, space);

			buffer->read = buffer->memory;
			data_size -= space;
			data += space;

			// Read second portion
			memcpy(data, buffer->read, data_size);
		}
	} else {
		memcpy(data, buffer->read, data_size);
	}

	buffer->read += data_size;
	buffer->size -= data_size;

	return data_size;
}

int buffer_peak(Buffer* buffer, void* data, int data_size) {
	int bytes_read;
	int old_size;
	void* old_read;

	assert(buffer != NULL);
	assert(data != NULL);

	old_size = buffer->size;
	old_read = buffer->read;

	bytes_read = buffer_read(buffer, data, data_size);

	// Restore
	buffer->size = old_size;
	buffer->read = old_read;

	return bytes_read;
}

int buffer_skip(Buffer* buffer, int how_many) {
	assert(buffer != NULL);

	if (how_many <= 0) return 0;
	if (how_many > buffer->size) return 0;

	buffer->read += how_many;

	if (buffer->read >= buffer_end(buffer)) {
		buffer->read -= buffer->capacity;
	}

	return how_many;
}

void buffer_clear(Buffer* buffer) {
	buffer->read = buffer->memory;
	buffer->write = buffer->memory;
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

void* buffer_end(Buffer* buffer) {
	return buffer->memory + buffer->capacity;
}

int buffer_free_space(Buffer* buffer) {
	return buffer->capacity - buffer->size;
}

/******* PRIVATE *******/

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

int _enough_space(Buffer* buffer, int requested_size) {
	return requested_size <= buffer_free_space(buffer);
}

int _enough_data(Buffer* buffer, int requested_size) {
	return requested_size <= buffer->size;
}

double _now() {
	return ((double)clock()) / CLOCKS_PER_SEC;
}

int _escalation_level(Buffer* buffer, double start_time) {
	double elapsed = _now() - start_time;

	if (buffer_has_timeout(buffer) && elapsed > buffer->timeout) {
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
		switch (_escalation_level(buffer, start_time)) {
			case LEVEL_ONE: break;
			case LEVEL_TWO: sched_yield(); break;
			case LEVEL_THREE: usleep(100000); break;
			case TIMEOUT: return -1;
		}
	}

	return 0;
}
