#ifndef IPC_BENCH_PROCESS_H
#define IPC_BENCH_PROCESS_H

char *find_build_path();

void start_process(char *argv[]);

void copy_arguments(char *arguments[], int argc, char *argv[]);

void start_child(char *name, int argc, char *argv[]);

void start_children(char *prefix, int argc, char *argv[]);

#endif /* IPC_BENCH_PROCESS_H */
