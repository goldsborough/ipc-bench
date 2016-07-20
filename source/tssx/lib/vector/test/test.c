#include <assert.h>
#include <stdio.h>

#include "vector.h"

void test_assert(bool b) {
	if(!b) {
		perror("assertion failed in test\n");
		exit(-1);
	}
}

int main(int argc, const char* argv[]) {
	int i;

	printf("TESTING SETUP ...\n");
	Vector vector = VECTOR_INITIALIZER;
	test_assert(!vector_is_initialized(&vector));
	vector_setup(&vector, 0, sizeof(int));
	test_assert(vector_is_initialized(&vector));

	printf("TESTING INSERT ...\n");

	for (i = 0; i < 1000; ++i) {
		test_assert(vector_insert(&vector, 0, &i) == VECTOR_SUCCESS);
		test_assert(VECTOR_GET_AS(int, &vector, 0) == i);
		test_assert(vector.size == i + 1);
	}

	int x = 5;
	vector_push_back(&vector, &x);
	vector_insert(&vector, vector.size, &x);

	printf("TESTING ASSIGNMENT ...\n");

	for (i = 0; i < vector.size; ++i) {
		int value = 666;
		test_assert(vector_assign(&vector, i, &value) == VECTOR_SUCCESS);
	}

	printf("TESTING ITERATION ...\n");

	VECTOR_FOR_EACH(&vector, iterator) {
		test_assert(ITERATOR_GET_AS(int, &iterator) == 666);
	}

	printf("TESTING REMOVAL ...\n");

	size_t expected_size = vector.size;
	while (!vector_is_empty(&vector)) {
		test_assert(vector_erase(&vector, 0) == VECTOR_SUCCESS);
		test_assert(vector.size == --expected_size);
	}

	printf("TESTING RESIZE ...\n");
	test_assert(vector_resize(&vector, 100) == VECTOR_SUCCESS);
	test_assert(vector.size == 100);
	test_assert(vector.capacity > 100);

	printf("TESTING RESERVE ...\n");
	test_assert(vector_reserve(&vector, 200) == VECTOR_SUCCESS);
	test_assert(vector.size == 100);
	test_assert(vector.capacity == 200);

	printf("TESTING CLEAR ...\n");
	test_assert(vector_clear(&vector) == VECTOR_SUCCESS);

	test_assert(vector.size == 0);
	test_assert(vector_is_empty(&vector));
	test_assert(vector.capacity == VECTOR_MINIMUM_CAPACITY);

	vector_destroy(&vector);

	printf("\033[92mALL TEST PASSED\033[0m\n");
}
