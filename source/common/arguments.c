#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common/arguments.h"

#define true 1
#define false 0

void print_usage() {
	printf(
			"Usage: fifos "
			"-s/--size <bytes> "
			"-c/--count <number>"
			"\n");
	exit(EXIT_FAILURE);
}

void parse_arguments(Arguments *arguments, int argc, char *argv[]) {
	// For getopt long options
	int long_index = 0;
	// For getopt chars
	int option;

	// Reset the option index to 1 if it
	// was modified before (e.g. in check_flag)
	optind = 0;

	// Default values
	arguments->size = DEFAULT_MESSAGE_SIZE;
	arguments->count = 1000;

	// Command line arguments
	// clang-format off
	static struct option long_options[] = {
			{"size",  required_argument, NULL, 's'},
			{"count", required_argument, NULL, 'c'},
			{0,       0,                 0,     0}
	};
	// clang-format on

	while (true) {
		option = getopt_long(argc, argv, "+:s:c:", long_options, &long_index);

		switch (option) {
			case -1: return;
			case 's': arguments->size = atoi(optarg); break;
			case 'c': arguments->count = atoi(optarg); break;
			default: continue;
		}
	}
}

int check_flag(const char *flag, int argc, char *argv[]) {
	// For getopt long options
	int index = 0;
	// The char returned by getopt()
	int option;
	// Setting the first character to be a plus
	// prevents getopt() from reordering the vector,
	// setting the second character to be a colon
	// prevents getopt() from printing an error
	// message when it encounters invalid options
	char short_flag[3] = {'+', ':', flag[0]};

	// Reset getopt index
	optind = 0;

	// clang-format off
	struct option long_options [2] = {
		{flag, no_argument, NULL, flag[0]},
		{0,    0,           0,    0}
	};
	// clang-format on

	while (true) {
		option = getopt_long(argc, argv, short_flag, long_options, &index);

		if (option == flag[0]) {
			return true;
		} else if (option == -1) {
			break;
		}
	}

	return false;
}
