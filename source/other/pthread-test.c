#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define true 1
#define false 0

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_variable = PTHREAD_COND_INITIALIZER;

int sum = 0;
int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
int i = 0;


struct Task {
	int first;
	int last;
};

void create_thread(pthread_t* thread,
									 void* (*function)(void*),
									 void* argument) {
	if (pthread_create(thread, NULL, function, argument) != 0) {
		perror("Error creating thread");
		exit(EXIT_FAILURE);
	}
}

void add(struct Task* task) {
	int i;
	for (i = task->first; i < task->last; ++i) {
		pthread_mutex_lock(&mutex);
		sum += array[i];
		pthread_mutex_unlock(&mutex);
	}
}

void parallel_add() {
	pthread_t first_thread;
	pthread_t second_thread;

	struct Task first_task = {0, 5};
	struct Task second_task = {first_task.last, 10};

	create_thread(&first_thread, (void*)add, &first_task);
	create_thread(&second_thread, (void*)add, &second_task);

	pthread_join(first_thread, NULL);
	pthread_join(second_thread, NULL);
}

void first_interleaved_half(struct Task* task) {
	pthread_mutex_lock(&mutex);
	printf("A\n");
	for (i = task->first; i < task->last; ++i) {
		printf("A: %d\n", i);
		sum += array[i];
	}

	pthread_mutex_unlock(&mutex);

	pthread_cond_broadcast(&condition_variable);

	pthread_exit(NULL);
}

void second_interleaved_half(struct Task* task) {
	pthread_mutex_lock(&mutex);
	printf("B\n");

	pthread_cond_wait(&condition_variable, &mutex);

	for (; i < task->last; ++i) {
		printf("B: %d\n", i);
		sum += array[i];
	}

	pthread_mutex_unlock(&mutex);
	pthread_exit(NULL);
}

void interleaved_add() {
	pthread_t first_thread;
	pthread_t second_thread;
	struct Task first_task = {0, 5};
	struct Task second_task = {5, 10};

	create_thread(&first_thread, (void*)first_interleaved_half, &first_task);
	create_thread(&second_thread, (void*)second_interleaved_half, &second_task);

	pthread_join(first_thread, NULL);
	pthread_join(second_thread, NULL);
}

void recursive_addition(struct Task* task) {
	if ((task->last - task->first) <= 2) {
		for (; task->first < task->last; ++task->first) {
			pthread_mutex_lock(&mutex);
			sum += array[task->first];
			pthread_mutex_unlock(&mutex);
		}

	} else {
		pthread_t thread;
		struct Task left_task = {task->first,
														 task->first + (task->last - task->first) / 2};
		struct Task right_task = {left_task.last, task->last};

		create_thread(&thread, (void*)recursive_addition, &left_task);

		recursive_addition(&right_task);

		pthread_join(thread, NULL);
	}
}

void recursive_add() {
	struct Task task = {0, 10};
	recursive_addition(&task);
}

int main() {
	// parallel_add();
	// interleaved_add();
	recursive_add();

	printf("%d\n", sum);

	return EXIT_SUCCESS;
}
