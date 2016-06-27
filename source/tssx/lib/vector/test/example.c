#include "vector.h"

int main(int argc, const char* argv[]) {
	Vector vector;
	int x, y, sum;

	/* Choose initial capacity of 10 */
	/* Specify the size of the elements you want to store once */
	vector_setup(&vector, 10, sizeof(int));

	x = 6, y = 9;
	vector_push_back(&vector, &x);
	vector_insert(&vector, 0, &y);
	vector_assign(&vector, 0, &y);

	x = *(int*)vector_get(&vector, 0);
	y = *(int*)vector_back(&vector);
	x = VECTOR_GET_AS(int, &vector, 1);

	vector_erase(&vector, 1);

	/* Iterator support */
	Iterator iterator = vector_begin(&vector);
	Iterator last = vector_end(&vector);
	for (; !iterator_equals(&iterator, &last); iterator_increment(&iterator)) {
		*(int*)iterator_get(&iterator) += 1;
	}

	/* Or just use pretty macros */
	sum = 0;
	VECTOR_FOR_EACH(&vector, i) {
		sum += ITERATOR_GET_AS(int, &i);
	}

	/* Memory management interface */
	vector_resize(&vector, 10);
	vector_reserve(&vector, 100);

	vector_clear(&vector);
	vector_destroy(&vector);
}
