#ifndef BUFFER_H
#define BUFFER_H

#define FOREVER 1
#define ERROR -1

#define LEVEL_ONE_TIME 0.001
#define LEVEL_TWO_TIME 0.01

#define TIMEOUT -1
#define LEVEL_ONE 1
#define LEVEL_TWO 2
#define LEVEL_THREE 3

typedef struct Buffer {
	// The current size of the buffer (number of bytes used)
	int size;

	// The length of the memory segment in bytes
	int capacity;

	// The blocking timeout, in fractional seconds.
	double timeout;

	// The read index
	int read;

	// The write index
	int  write;

} Buffer;


Buffer*
create_buffer(void* shared_memory, int requested_capacity, double timeout);

int buffer_write(Buffer* buffer, void* data, int data_size);
int buffer_read(Buffer* buffer, void* data, int data_size);

int buffer_peek(Buffer* buffer, void* data, int data_size);
int buffer_skip(Buffer* buffer, int how_many);

void buffer_clear(Buffer* buffer);

int buffer_is_full(Buffer* buffer);
int buffer_is_empty(Buffer* buffer);
int buffer_has_timeout(Buffer* buffer);

int buffer_free_space(Buffer* buffer);

/******* PRIVATE *******/

typedef int (*Condition)(Buffer*, int);

void* _start_pointer(Buffer* buffer);
void* _end_pointer(Buffer* buffer);
void* _read_pointer(Buffer* buffer);
void* _write_pointer(Buffer* buffer);

void* _pointer_to(Buffer* buffer, int index);
int _index_at(Buffer* buffer, void* pointer);

void _wrap_read(Buffer* buffer, void** data, int* data_size, int delta);
void _wrap_write(Buffer* buffer, void** data, int* data_size, int delta);
void _reduce_data(void** data, int* data_size, int delta);

void _check_write_error(int return_code);
void _check_read_error(int return_code);

int _writeable(Buffer* buffer, int requested_size);
int _readable(Buffer* buffer, int requested_size);

double _now();

int _escalation_level(Buffer* buffer, double start_time);

int _block(Buffer* buffer, int requested_size, Condition predicate);

#endif /* BUFFER_H */
