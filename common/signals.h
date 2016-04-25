#ifndef IPC_BENCH_SIGNALS_H
#define IPC_BENCH_SIGNALS_H

#define IGNORE_USR1 0x0
#define IGNORE_USR2 0x0
#define BLOCK_USR1 0x1
#define BLOCK_USR2 0x2

struct sigaction;

void signal_handler(int _);

void setup_signals(struct sigaction *signal_action, int flags);

void setup_parent_signals();

void setup_server_signals(struct sigaction *signal_action);

void setup_client_signals(struct sigaction *signal_action);

void client_signal();

void server_signal();

void wait_for_signal(struct sigaction *signal_action);

#endif /* IPC_BENCH_SIGNALS_H */
