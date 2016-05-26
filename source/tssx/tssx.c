#include "tssx/tssx.h"
#include "tssx/hashtable.h"

struct HashTable connection_map;

void tssx() {
	ht_setup(&connection_map, 0);
}
