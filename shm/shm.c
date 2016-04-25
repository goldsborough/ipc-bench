#include "common/process.h"
#include "common/signals.h"

int main(int argc, char* argv[]) {
	setup_parent_signals();
	start_children("shm", argc, argv);
}
