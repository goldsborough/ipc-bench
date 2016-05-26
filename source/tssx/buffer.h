#ifndef BUFFER_H
#define BUFFER_H

typedef struct Buffer {
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

} Buffer;

Buffer* create_buffer(void* shared_memory, int requested_capacity);

int buffer_write(Buffer* buffer, void* data, int data_size);
int buffer_read(Buffer* buffer, void* data, int data_size);

int buffer_peak(Buffer* buffer, void* data, int data_size);
int buffer_skip(Buffer* buffer, int how_many);

void clear_buffer(Buffer* buffer);

int buffer_is_full(Buffer* buffer);
int buffer_is_empty(Buffer* buffer);

void check_write_error(int return_code);
void check_read_error(int return_code);

void* buffer_end(Buffer* buffer);

#endif /* BUFFER_H */
