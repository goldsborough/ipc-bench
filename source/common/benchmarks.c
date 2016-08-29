#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "common/arguments.h"
#include "common/benchmarks.h"

bench_t now() {
#ifdef __MACH__
	return ((double)clock()) / CLOCKS_PER_SEC * 1e9;
#else
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);

	return ts.tv_sec * 1e9 + ts.tv_nsec;

#endif
}

void setup_benchmarks(Benchmarks* bench) {
	bench->minimum = INT32_MAX;
	bench->maximum = 0;
	bench->sum = 0;
	bench->squared_sum = 0;
	bench->total_start = now();
}

void benchmark(Benchmarks* bench) {
	const bench_t time = now() - bench->single_start;

	if (time < bench->minimum) {
		bench->minimum = time;
	}

	if (time > bench->maximum) {
		bench->maximum = time;
	}

	bench->sum += time;
	bench->squared_sum += (time * time);
}

void evaluate(Benchmarks* bench, Arguments* args) {
	assert(args->count > 0);
	const bench_t total_time = now() - bench->total_start;
	const double average = ((double)bench->sum) / args->count;

	double sigma = bench->squared_sum / args->count;
	sigma = sqrt(sigma - (average * average));

	int messageRate = (int)(args->count / (total_time / 1e9));

	printf("\n============ RESULTS ================\n");
	printf("Message size:       %d\n", args->size);
	printf("Message count:      %d\n", args->count);
	printf("Total duration:     %.3f\tms\n", total_time / 1e6);
	printf("Average duration:   %.3f\tus\n", average / 1000.0);
	printf("Minimum duration:   %.3f\tus\n", bench->minimum / 1000.0);
	printf("Maximum duration:   %.3f\tus\n", bench->maximum / 1000.0);
	printf("Standard deviation: %.3f\tus\n", sigma / 1000.0);
	printf("Message rate:       %d\tmsg/s\n", messageRate);
	printf("=====================================\n");
}
