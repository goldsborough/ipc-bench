#ifndef TIMEOUTS_H
#define TIMEOUTS_H

#include <stdint.h>

typedef uint64_t cycle_t;

#define INFINITE 0

#define DEFAULT_LEVEL_ZERO_CLOCKS (1000)
#define DEFAULT_LEVEL_ONE_CLOCKS (100 * 1000)

// clang-format off
#define DEFAULT_TIMEOUTS_INITIALIZER	\
{                                      \
	{												\
		DEFAULT_LEVEL_ZERO_CLOCKS,			\
		DEFAULT_LEVEL_ONE_CLOCKS			\
	},												\
	INFINITE										\
}
// clang-format on

typedef struct Timeouts {
	cycle_t levels[2];
	cycle_t timeout;
} Timeouts;

extern const Timeouts DEFAULT_TIMEOUTS;

#endif /* TIMEOUTS_H */
