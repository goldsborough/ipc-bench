#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "common/arguments.h"
#include "common/benchmarks.h"

long now() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

void setup_benchmarks(struct Benchmarks *bench) {
	bench->minimum = INT32_MAX;
	bench->maximum = 0;
	bench->sum = 0;
	bench->squared_sum = 0;
	bench->total_start = now();
}

void benchmark(struct Benchmarks *bench) {
	const long time = now() - bench->single_start;

	if (time < bench->minimum) {
		bench->minimum = time;
	}

	if (time > bench->maximum) {
		bench->maximum = time;
	}

	bench->sum += time;
	bench->squared_sum += (time * time);
}

void evaluate(struct Benchmarks *bench, struct Arguments *args) {
	assert(args->count > 0);
	const long total_time = now() - bench->total_start;
	const double average = ((double)bench->sum) / args->count;

	double sigma = bench->squared_sum / args->count;
	sigma = sqrt(sigma - (average * average));


	printf("\n============ RESULTS =============\n");
	printf("Message size:       %d\n", args->size);
	printf("Message count:      %d\n", (int)args->count);
	printf("Total duration:     %.3f\tms\n", total_time / 1000 / 1000.0);
	printf("Average duration:   %.3f\tus\n", average / 1000.0);
	printf("Minimum duration:   %.3f\tus\n", bench->minimum / 1000.0);
	printf("Maximum duration:   %.3f\tus\n", bench->maximum / 1000.0);
	printf("Standard deviation: %.3f\tus\n", sigma);
	printf("EasyToPlot min %.3f\n us", bench->minimum / 1000.0);
	printf("==================================\n");
}
