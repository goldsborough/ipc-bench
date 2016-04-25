#include <assert.h>
#include <getopt.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define true 1
#define false 0

void throw(const char *message) {
	perror(message);
	exit(EXIT_FAILURE);
}

void command_line_error() {
	printf(
			"Usage: fifos "
			"-s/--size <bytes> "
			"-c/--count <number>"
			"\n");
	exit(EXIT_FAILURE);
}

struct Arguments {
	int size;
	int count;
};

void parse_arguments(struct Arguments *arguments, int argc, char *argv[]) {
	// For getopt long options
	int long_index = 0;
	// For getopt chars
	int option;

	// Default values
	arguments->size = getpagesize();
	arguments->count = 1000;

	// Command line arguments
	// clang-format off
	static struct option long_options[] = {
			{"size",  required_argument, NULL, 's'},
			{"count", required_argument, NULL, 'c'},
			{"help",  no_argument,       NULL, 'h'},
			{0,       0,                 0,     0}
	};

	while ((option = getopt_long(
						argc,
						argv,
						"s:c:h",
						long_options,
							&long_index)) != -1)
		{
		// clang-format on
		switch (option) {
			case 's': arguments->size = atoi(optarg); break;
			case 'c': arguments->count = atoi(optarg); break;
			case 'h':
			default: command_line_error();
		}
	}
}

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

void start_process(char *argv[]) {
	// Will need to set the group id
	const pid_t parent_pid = getpid();
	const pid_t pid = fork();

	if (pid == 0) {
		// Set group id of the children so that we
		// can send around signals
		if (setpgid(pid, parent_pid) == -1) {
			throw("Could not set group id for child process");
		}
		// Replace the current process with the command
		// we want to execute (child or server)
		// First argument is the command to call,
		// second is an array of arguments, where the
		// command path has to be included as well
		// (that's why argv[0] first)
		if (execv(argv[0], argv) == -1) {
			throw("Error opening child process");
		}
	}
}

void copy_arguments(char *arguments[], int argc, char *argv[]) {
	int i;
	assert(argc < 8);
	for (i = 1; i < argc; ++i) {
		arguments[i] = argv[i];
	}

	arguments[argc] = NULL;
}

void start_server(int argc, char *argv[]) {
	char *arguments[8] = {"./server"};
	copy_arguments(arguments, argc, argv);
	start_process(arguments);
}

void start_client(int argc, char *argv[]) {
	char *arguments[8] = {"./client"};
	copy_arguments(arguments, argc, argv);
	start_process(arguments);
}

void signal_handler(int _) {
}

void setup_server_signals(struct sigaction *signal_action) {
	signal_action->sa_flags = SA_RESTART;
	signal_action->sa_handler = signal_handler;
	sigaddset(&signal_action->sa_mask, SIGUSR1);
	sigprocmask(SIG_BLOCK, &signal_action->sa_mask, NULL);

	if (sigaction(SIGUSR2, signal_action, NULL)) {
		throw("Error registering signal handler for server");
	}
}

void setup_client_signals(struct sigaction *signal_action) {
	signal_action->sa_flags = SA_RESTART;
	signal_action->sa_handler = signal_handler;
	sigaddset(&signal_action->sa_mask, SIGUSR2);
	sigprocmask(SIG_BLOCK, &signal_action->sa_mask, NULL);

	if (sigaction(SIGUSR1, signal_action, NULL)) {
		throw("Error registering signal handler for client");
	}
}

void client_signal() {
	kill(0, SIGUSR1);
}

void server_signal() {
	kill(0, SIGUSR2);
}

void wait_for_signal(struct sigaction *signal_action) {
	int signal_number;
	sigwait(&signal_action->sa_mask, &signal_number);
}
