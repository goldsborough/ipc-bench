#include <assert.h>
#include <string.h>

#include "buffer.h"
#include "common/utility.h"

struct Buffer* create_buffer(void* shared_memory, int requested_capacity) {
	struct Buffer* buffer = (struct Buffer*)shared_memory;

	buffer->memory = buffer + sizeof(struct Buffer);
	buffer->capacity = requested_capacity;

	clear_buffer(buffer);

	return buffer;
}

int buffer_write(struct Buffer* buffer, void* data, int data_size) {
	int space;
	int return_code;

	assert(buffer != NULL);
	assert(data != NULL);

	if (data_size <= 0) return 0;
	if (data_size > (buffer->capacity - buffer->size)) return 0;

	// The == is when the buffer is empty
	if (buffer->write >= buffer->read) {
		// Available space to the right of the write pointer
		space = buffer_end(buffer) - buffer->write;

		// Enough forward space
		if (data_size <= space) {
			return_code = memcpy_s(buffer->write, space, data, data_size);
			check_write_error(return_code);

		} else {
			// Write first portion, up to the end of the buffer
			return_code = memcpy_s(buffer->write, space, data, space);
			check_write_error(return_code);

			buffer->write = buffer->memory;
			data_size -= space;
			data += space;
			space = buffer->read - buffer->memory;

			// Write seocnd portion
			memcpy_s(buffer->write, space, data, data_size);
			check_write_error(return_code);
		}
	} else {
		space = buffer->read - buffer->write;

		memcpy_s(buffer->write, space, data, data_size);
		check_write_error(return_code);
	}

	buffer->write += data_size;
	buffer->size += data_size;

	return data_size;
}

int buffer_read(struct Buffer* buffer, void* data, int data_size) {
	int return_code;

	assert(buffer != NULL);
	assert(data != NULL);

	if (data_size <= 0) return 0;
	if (data_size > buffer->size) return 0;

	if (buffer->read >= buffer->write) {
		// Available space to the right of the write pointer
		space = buffer_end(buffer) - buffer->write;

		if (data_size <= space) {
			return_code = memcpy_s(data, data_size, buffer->read, data_size);
			check_read_error(return_code);
		} else {
			// Read first portion, up to the end of the buffer
			return_code = memcpy_s(data, data_size, buffer->read, space);
			check_read_error(return_code);

			buffer->read = buffer->memory;
			data_size -= space;
			data += space;

			// Read second portion
			memcpy_s(data, data_size, buffer->read, data_size);
			check_read_error(return_code);
		}
	} else {
		memcpy_s(data, data_size, buffer->read, data_size);
		check_read_error(return_code);
	}

	buffer->read += data_size;
	buffer->size -= data_size;

	return data_size;
}

int buffer_peak(struct Buffer* buffer, void* data, int data_size) {
	int bytes_read;
	int old_size;
	void* old_read;

	assert(buffer != NULL);

	old_size = buffer->size;
	old_read = buffer->read;

	bytes_read = read_from_buffer(buffer, data, data_size);

	buffer->size = old_size;
	buffer->read = old_read;

	return bytes_read;
}

int buffer_skip(struct Buffer* buffer, int how_many) {
	assert(buffer != NULL);
	assert(data != NULL);

	if (how_many <= 0) return 0;
	if (how_many > buffer->size) return 0;

	buffer->read += how_many;

	if (buffer->read >= end(buffer)) {
		buffer->read -= buffer->capacity;
	}

	return how_many;
}

void clear_buffer(struct Buffer* buffer) {
	buffer->read = buffer->memory;
	buffer->write = buffer->memory;
	buffer->size = 0;
}

int buffer_is_full(struct Buffer* buffer) {
	return buffer->size == buffer->capacity;
}

int buffer_is_empty(struct Buffer* buffer) {
	return buffer->size == 0;
}

void check_write_error(int return_code) {
	if (return_code != 0) {
		throw("Error writing to buffer");
	}
}

void check_read_error(int return_code) {
	if (return_code != 0) {
		throw("Error reading from buffer");
	}
}

void* buffer_end(struct Buffer* buffer) {
	return buffer->memory + buffer->capacity;
}
