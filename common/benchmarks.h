#ifndef IPC_BENCH_BENCHMARKS_H
#define IPC_BENCH_BENCHMARKS_H

struct Arguments;

struct Benchmarks {
	// Start of the total benchmarking
	double total_start;

	// Start of single benchmark
	double single_start;

	// Minimum time
	double minimum;

	// Maximum time
	double maximum;

	// Sum (for averaging)
	double sum;

	// Squared sum (for standard deviation)
	double squared_sum;
};

double now();

void setup_benchmarks(struct Benchmarks *bench);

void benchmark(struct Benchmarks *bench);

void evaluate(struct Benchmarks *bench, struct Arguments *args);

#endif /* IPC_BENCH_BENCHMARKS_H */
