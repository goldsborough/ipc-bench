#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

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
	} else if (time > bench->maximum) {
		bench->maximum = time;
	}

	bench->sum += time;
	bench->squared_sum += (time * time);
}

void evaluate(struct Benchmarks *bench, int size, double count) {
	const int total_time = now() - bench->total_start;
	const double average = bench->sum / count;
	const double sigma = sqrt((bench->squared_sum / count) - (average * average));

	printf("\n============ RESULTS =============\n");
	printf("Message size:       %d\n", size);
	printf("Message count:      %d\n", (int)count);
	printf("Total duration:     %d\tms\n", total_time / 1000);
	printf("Average duration:   %.3f\tus\n", average);
	printf("Minimum duration:   %d\t\tus\n", bench->minimum);
	printf("Maximum duration:   %d\t\tus\n", bench->maximum);
	printf("Standard deviation: %.3f\tus\n", sigma);
	printf("==================================\n");
}
