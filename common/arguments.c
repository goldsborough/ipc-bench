#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common/arguments.h"

void command_line_error() {
	printf(
			"Usage: fifos "
			"-s/--size <bytes> "
			"-c/--count <number>"
			"\n");
	exit(EXIT_FAILURE);
}

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
