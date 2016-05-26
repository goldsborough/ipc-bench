#include <stdio.h>

#include "connection.h"
#include "hashtable.h"

int main(int argc, const char* argv[]) {
	struct HashTable table;
	struct Connection a, b, c;

	ht_setup(&table, 0);

	a.segment_id = 0;
	b.segment_id = 1;
	c.segment_id = 2;

	ht_insert(&table, &a);
	ht_insert(&table, &b);
	ht_insert(&table, &c);

	printf("%d\n", ht_get(&table, 0)->segment_id);
	printf("%d\n", ht_get(&table, 1)->segment_id);
	printf("%d\n", ht_get(&table, 2)->segment_id);

	ht_destroy(&table);
}
