#ifndef BUFFER_H
#define BUFFER_H

#define FOREVER 1

#define LEVEL_ONE_TIME 0.01
#define LEVEL_TWO_TIME 1.00

#define TIMEOUT -1
#define LEVEL_ONE 1
#define LEVEL_TWO 2
#define LEVEL_THREE 3

typedef struct Buffer {
	// The pointer to the first byte of the buffer
	void* memory;

	// The current size of the buffer (number of bytes used)
	int size;

	// The length of the memory segment in bytes
	int capacity;

	// The blocking timeout, in fractional seconds.
	double timeout;

	// The read pointer
	void* read;

	// The write pointer
	void* write;

} Buffer;


Buffer*
create_buffer(void* shared_memory, int requested_capacity, double timeout);

int buffer_write(Buffer* buffer, void* data, int data_size);
int buffer_read(Buffer* buffer, void* data, int data_size);

int buffer_peak(Buffer* buffer, void* data, int data_size);
int buffer_skip(Buffer* buffer, int how_many);

void buffer_clear(Buffer* buffer);

int buffer_is_full(Buffer* buffer);
int buffer_is_empty(Buffer* buffer);
int buffer_has_timeout(Buffer* buffer);

int buffer_free_space(Buffer* buffer);

void* buffer_end(Buffer* buffer);

/******* PRIVATE *******/

typedef int (*Condition)(Buffer*, int);

void _check_write_error(int return_code);
void _check_read_error(int return_code);

int _enough_space(Buffer* buffer, int requested_size);
int _enough_data(Buffer* buffer, int requested_size);

double _now();

int _escalation_level(Buffer* buffer, double start_time);

int _block(Buffer* buffer, int requested_size, Condition predicate);

#endif /* BUFFER_H */
