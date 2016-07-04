#ifndef TIMEOUTS_H
#define TIMEOUTS_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

/******************** DEFINITIONS ********************/

typedef uint64_t cycle_t;

#define INFINITE 0

// Different timeout levels
#define TIMEOUT -1
#define LEVEL_ZERO 0
#define LEVEL_ONE 1
#define LEVEL_TWO 2

// Conversion notes:
// 1 clock cycle ~ 0.25 ns (assuming 4Ghz = 1/(4 * 10^9))
// 1000 clock cycles ~ 250 ns
// 1 pause instruction ~ 40 clock cyles ~ 10 ns
// 1000 clock cycles ~ 250 ns ~ 25 pause instructions

#define DEFAULT_LEVEL_ZERO_CLOCKS (10 * 000) // ~2500 ns, 250 pauses
#define DEFAULT_LEVEL_ONE_CLOCKS (100 * 1000)// ~25 us

#define SECONDS_TO_CLOCKS(seconds) ((cycle_t)((seconds)*CLOCKS_PER_SEC))
#define CLOCKS_TO_SECONDS(clock_cycles) (((double)seconds) / CLOCKS_PER_SEC)

// clang-format off
#ifdef TSSX_SUPPORT_BUFFER_TIMEOUTS
#define DEFAULT_TIMEOUTS_INITIALIZER \
{ \
   {false, false}, \
  { DEFAULT_LEVEL_ZERO_CLOCKS, DEFAULT_LEVEL_ONE_CLOCKS }, \
  {INFINITE, INFINITE } \
}
#else
#define DEFAULT_TIMEOUTS_INITIALIZER \
{ \
   {false, false}, \
  { DEFAULT_LEVEL_ZERO_CLOCKS, DEFAULT_LEVEL_ONE_CLOCKS } \
}
#endif
// clang-format on

/******************** STRUCTURES ********************/

typedef struct Timeouts {
	bool non_blocking[2];
	cycle_t levels[2];

#ifdef TSSX_SUPPORT_BUFFER_TIMEOUTS
	cycle_t timeout[2];
#endif
} Timeouts;

extern const Timeouts DEFAULT_TIMEOUTS;

/******************** INTERFACE ********************/


#endif /* TIMEOUTS_H */
