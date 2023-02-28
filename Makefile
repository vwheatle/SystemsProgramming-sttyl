CC = gcc
CC_FLAGS = -std=c99 -Wall -Wpedantic -Wextra -fsanitize=undefined
VALGRIND_FLAGS = --quiet --tool=memcheck --leak-check=yes --show-reachable=yes --num-callers=3 --error-exitcode=1

all: sttyl

clean:
	rm -f sttyl

sttyl: sttyl.c
	$(CC) $(CC_FLAGS) -o sttyl $^
# -g for debugging with gdb...

run:
	@valgrind $(VALGRIND_FLAGS) ./sttyl
