#include <stdio.h>
#include <sys/eventfd.h>

void communicate(int eventfd, struct Arguments* args) {
	struct Benchmarks bench;
	int message;

	setup_benchmarks(&bench);

	for (message = 0; message < args->count; ++message) {
	}
}

int main(int argc, char* argv[]) {
	struct Arguments args;
	parse_arguments(&args, argc, argv);
}
