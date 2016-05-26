#ifndef BUFFER_H
#define BUFFER_H

struct Buffer {
	// The pointer to the first byte of the buffer
	void* memory;

	// The current size of the buffer (number of bytes used)
	int size;

	// The length of the memory segment in bytes
	int capacity;

	// The read pointer
	void* read;

	// The write pointer
	void* write;
};

struct Buffer* create_buffer(void* shared_memory, int requested_capacity);

int buffer_write(struct Buffer* buffer, void* data, int data_size);
int buffer_read(struct Buffer* buffer, void* data, int data_size);

int buffer_peak(struct Buffer* buffer, void* data, int data_size);
int buffer_skip(struct Buffer* buffer, int how_many);

void clear_buffer(struct Buffer* buffer);

int buffer_is_full(struct Buffer* buffer);
int buffer_is_empty(struct Buffer* buffer);

void check_write_error(int return_code);
void check_read_error(int return_code);

void* buffer_end(struct Buffer* buffer);

#endif /* BUFFER_H */
