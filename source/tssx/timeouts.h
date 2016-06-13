#ifndef TIMEOUTS_H
#define TIMEOUTS_H

#include <stdint.h>

typedef uint64_t cycle_t;

#define INFINITE 0

#define DEFAULT_LEVEL_ZERO_TIME 0.001
#define DEFAULT_LEVEL_ONE_TIME 0.01

#define CONVERT_TO_CLOCK_CYCLES(seconds) ((cycle_t)((seconds)*CLOCKS_PER_SEC))
#define CONVERT_TO_SECONDS(clock_cycles) (((double)seconds) / CLOCKS_PER_SEC)

// clang-format off
#define DEFAULT_TIMEOUTS_INITIALIZER									\
{                                                     \
	{																										\
		CONVERT_TO_CLOCK_CYCLES(DEFAULT_LEVEL_ZERO_TIME),	\
		CONVERT_TO_CLOCK_CYCLES(DEFAULT_LEVEL_ZERO_TIME)	\
	},																									\
	INFINITE																						\
}
// clang-format on

typedef struct Timeouts {
	cycle_t levels[2];
	cycle_t timeout;
} Timeouts;

extern const Timeouts DEFAULT_TIMEOUTS;

Timeouts create_timeouts(double timeout_seconds);

#endif /* TIMEOUTS_H */
