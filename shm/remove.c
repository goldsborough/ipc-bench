#include <sys/shm.h>

int main() {
	shmctl(7000, IPC_RMID, 0);
}
