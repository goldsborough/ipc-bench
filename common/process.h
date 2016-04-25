#ifndef IPC_BENCH_PROCESS_H
#define IPC_BENCH_PROCESS_H

void start_process(char *argv[]);

void copy_arguments(char *arguments[], int argc, char *argv[]);

void start_server(char *name, int argc, char *argv[]);

void start_client(char *name, int argc, char *argv[]);

void start_children(char *name, int argc, char *argv[]);

#endif /* IPC_BENCH_PROCESS_H */
