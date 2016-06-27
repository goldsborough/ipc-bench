#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

struct Buffer;

// Returns the ID
int create_segment(int total_size);

// Returns the shared memory
void* attach_segment(int segment_id);

void detach_segment(void* shared_memory);

void destroy_segment(int segment_id);

int segment_size(struct Buffer* buffer);

#endif /* SHARED_MEMORY_H */
