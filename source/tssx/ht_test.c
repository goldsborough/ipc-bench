#include <stdio.h>

#include "connection.h"
#include "hashtable.h"

int main(int argc, const char* argv[]) {
	struct HashTable table;
	struct Connection a, b, c;

	ht_setup(&table, 0);

	a.segment_id = 123;
	b.segment_id = 456;
	c.segment_id = 789;

	ht_insert(&table, 0, &a);
	ht_insert(&table, 1, &b);
	ht_insert(&table, 2, &c);

	printf("%d\n", ht_get(&table, 0)->segment_id);
	printf("%d\n", ht_get(&table, 1)->segment_id);
	printf("%d\n", ht_get(&table, 2)->segment_id);

	ht_destroy(&table);
}
