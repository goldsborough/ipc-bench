#ifndef IPC_BENCH_UTILITY_H
#define IPC_BENCH_UTILITY_H

#ifdef DEBUG
#include <stdio.h>
#endif /* DEBUG */

#include <stdbool.h>

/******************** DEFINITIONS ********************/

struct timeval;

/******************** INTERFACE ********************/

/**
 * Calls perror() and exits the program.
 *
 * Use this function when the error *is* the result of a syscall failure. Then
 * it will print the specified message along with the implementation defined
 * error message that is appended in perror(). Do not append a newline.
 *
 * \param message The message to print.
 *
 * \see terminate()
 */
void throw(const char* message) __attribute__((noreturn));

/**
 * Prints a message to stderr and exits the program.
 *
 * Use this function when the error is not the result of a syscall failure. Do
 * append a newline to the message.
 *
 * \param message The message to print.
 *
 * \see throw()
 */
void terminate(const char* message) __attribute__((noreturn));

/**
 * Prints a message to stderr.
 *
 * param message The message to print.
 */
void print_error(const char* message);

/**
 * Prints "Warning: <message>".
 *
 * \param message The warning message.
 */
void warn(const char* message);

int generate_key(const char* path);

void nsleep(int nanoseconds);

int current_milliseconds();
int timeval_to_milliseconds(const struct timeval* time);

void pin_thread(int where);

#endif /* IPC_BENCH_UTILITY_H */
