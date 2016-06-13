#include <time.h>

#include "tssx/timeouts.h"

const Timeouts DEFAULT_TIMEOUTS = DEFAULT_TIMEOUTS_INITIALIZER;

Timeouts create_timeouts(double timeout_seconds) {
	Timeouts timeouts = DEFAULT_TIMEOUTS_INITIALIZER;
	timeouts.seconds = CONVERT_TO_CLOCK_CYCLES(seconds);

	return timeotus;
}
