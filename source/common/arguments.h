#ifndef IPC_BENCH_ARGUMENTS_H
#define IPC_BENCH_ARGUMENTS_H

#define DEFAULT_MESSAGE_SIZE 4096

void print_usage();

typedef struct Arguments {
	int size;
	int count;

} Arguments;

void parse_arguments(Arguments* arguments, int argc, char* argv[]);

int check_flag(const char* name, int argc, char* argv[]);

#endif /* IPC_BENCH_ARGUMENTS_H */
