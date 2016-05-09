#ifndef IPC_BENCH_BENCHMARKS_H
#define IPC_BENCH_BENCHMARKS_H

struct Arguments;

struct Benchmarks {
	// Start of the total benchmarking
	long total_start;

	// Start of single benchmark
	long single_start;

	// Minimum time
	long minimum;

	// Maximum time
	long maximum;

	// Sum (for averaging)
	long sum;

	// Squared sum (for standard deviation)
	long squared_sum;
};

long now();

void setup_benchmarks(struct Benchmarks *bench);

void benchmark(struct Benchmarks *bench);

void evaluate(struct Benchmarks *bench, struct Arguments *args);

#endif /* IPC_BENCH_BENCHMARKS_H */
