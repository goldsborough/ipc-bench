#ifndef IPC_BENCH_BENCHMARKS_H
#define IPC_BENCH_BENCHMARKS_H

struct Benchmarks {
	// Start of the total benchmarking
	int total_start;

	// Start of single benchmark
	int single_start;

	// Minimum time
	int minimum;

	// Maximum time
	int maximum;

	// Sum (for averaging)
	int sum;

	// Squared sum (for standard deviation)
	int squared_sum;
};

double now();

void setup_benchmarks(struct Benchmarks *bench);

void benchmark(struct Benchmarks *bench);

void evaluate(struct Benchmarks *bench, int size, double count);

#endif /* IPC_BENCH_BENCHMARKS_H */
