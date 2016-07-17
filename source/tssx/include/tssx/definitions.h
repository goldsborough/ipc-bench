#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <stdint.h>

// Common definitions to avoid having to include
// whole headers when only these typedefs are needed

typedef int key_t;
typedef uint_fast64_t cycle_t;

typedef enum { SERVER, CLIENT } Side;

#endif /* DEFINITIONS_H */
