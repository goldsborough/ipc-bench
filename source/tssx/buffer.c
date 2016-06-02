#include <assert.h>
#include <string.h>

#include "common/utility.h"
#include "tssx/buffer.h"

Buffer* create_buffer(void* shared_memory, int requested_capacity) {
	Buffer* buffer = (Buffer*)shared_memory;

	buffer->memory = buffer + sizeof(Buffer);
	buffer->capacity = requested_capacity;

	clear_buffer(buffer);

	return buffer;
}

int buffer_write(Buffer* buffer, void* data, int data_size) {
	int space;

	assert(buffer != NULL);
	assert(data != NULL);

	if (data_size == 0) return 0;
	if (data_size > (buffer->capacity - buffer->size)) return 0;

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
	if (data_size > buffer->size) return 0;

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

void clear_buffer(Buffer* buffer) {
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

void* buffer_end(Buffer* buffer) {
	return buffer->memory + buffer->capacity;
}
