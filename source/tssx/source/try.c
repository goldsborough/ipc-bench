#include <stdio.h>
#include <stdlib.h>
#include "tssx/buffer.h"

int main(int argc, const char* argv[]) {
	int size = 1000000;
	void* memory = malloc(sizeof(Buffer) + size);
	int data;

	Buffer* buffer = create_buffer(memory, size, &DEFAULT_TIMEOUTS);

	cycle_t sum = 0;
	cycle_t n = 100000;
	for (int i = 0; i < n; ++i) {
		cycle_t start = _now();
		buffer_write(buffer, &data, sizeof data);
		// buffer_read(buffer, &data, sizeof data);
		sum += _now() - start;
	}

	printf("%llu\n", sum / n);

	free(memory);
}
