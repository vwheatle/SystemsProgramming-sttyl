// CS4-53203: Systems Programming
// Name: V Wheatley
// Date: 2023-01-30
// sttyl.c

// C stuff
#include <stdlib.h> // -> EXIT_*
#include <stdio.h> // buffered I/O

#include <string.h> // -> str*, mem*
#include <stdbool.h> // -> bool

// Linux stuff
#include <unistd.h> // syscalls
#include <termios.h> // terminal information

int main(int argc, char *argv[]) {
	if (argc <= 1) {
		char *name = (argc > 0) ? argv[0] : "sttyl";
		fprintf(stderr, "Usage: %s some thing\n", name);
		exit(EXIT_FAILURE);
	}
	
	struct termios terminalInfo;
	if (tcgetattr(STDIN_FILENO, &terminalInfo) == -1) {
		perror("oops");
		exit(EXIT_FAILURE);
	}
	
	return EXIT_SUCCESS;
}
