# vector

[![GitHub license](https://img.shields.io/github/license/mashape/apistatus.svg?style=flat-square)](http://goldsborough.mit-license.org)

`vector` is a feature-complete, generic and customizable resizable array implementation in pure C that supports almost the entire C++ `std::vector` API, including iterators.

## Usage

```C
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

	vector_remove(&vector, 1);

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
```

## LICENSE

This project is released under the [MIT License](http://goldsborough.mit-license.org). For more information, see the `LICENSE` file.

## Authors

[Peter Goldsborough](http://www.goldsborough.me) + [cat](https://goo.gl/IpUmJn) :heart:

<a href="https://gratipay.com/~goldsborough/"><img src="http://img.shields.io/gratipay/goldsborough.png?style=flat-square"></a>
