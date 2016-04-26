#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "common/arguments.h"
#include "common/benchmarks.h"

double now() {
	return ((double)clock()) / CLOCKS_PER_SEC * 1e6;
}

void setup_benchmarks(struct Benchmarks *bench) {
	bench->minimum = INT32_MAX;
	bench->maximum = 0;
	bench->sum = 0;
	bench->squared_sum = 0;
	bench->total_start = now();
}

void benchmark(struct Benchmarks *bench) {
	const int time = now() - bench->single_start;

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
	const double total_time = now() - bench->total_start;
	const double average = bench->sum / args->count;

	double sigma = bench->squared_sum / args->count;
	sigma = sqrt(sigma - (average * average));


	printf("============ RESULTS =============\n");
	printf("Message size:       %d\n", args->size);
	printf("Message count:      %d\n", (int)args->count);
	printf("Total duration:     %.3f\tms\n", total_time / 1000);
	printf("Average duration:   %.3f\tus\n", average);
	printf("Minimum duration:   %.3f\tus\n", bench->minimum);
	printf("Maximum duration:   %.3f\tus\n", bench->maximum);
	printf("Standard deviation: %.3f\tus\n", sigma);
	printf("==================================\n");
}
