V Wheatley  
Systems Programming

# `sttyl`

This document is going to be a bit more barebones than the previous. All code is also available in a [GitHub repository](https://github.com/vwheatle/SystemsProgramming-sttyl). (Aside: I was curious how font ligatures interacted with PDF export, and it seems they interact poorly! I've fixed it in my assignment template, but if you want the source code guaranteed intact and complete, the repository is the way to go.)

## `Makefile`

```makefile
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
```

## `sttyl.c`

```c
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

#define sizeofarr(arr) (sizeof(arr) / sizeof(*(arr)))
#define arrandsize(arr) arr, sizeofarr(arr)

enum bit_action { BIT_OFF = 0, BIT_ON = 1, BIT_INVERT = 2 };

char *get_baud_str(speed_t baud) {
	switch (baud) {
		case B0:     return "0";       case B50:    return "50";
		case B75:    return "75";      case B110:   return "110";
		case B134:   return "134";     case B150:   return "150";
		case B200:   return "200";     case B300:   return "300";
		case B600:   return "600";     case B1200:  return "1200";
		case B1800:  return "1800";    case B2400:  return "2400";
		case B4800:  return "4800";    case B9600:  return "9600";
		case B19200: return "19200";   case B38400: return "38400";
		default: return NULL;
	}
}

struct cc_info {
	int index;
	const char *name;
};
const struct cc_info LOOKUP_CONTROL_CHARS[] = {
	{ VINTR,  "intr"  },
	{ VERASE, "erase" },
	{ VKILL,  "kill"  },
	{ VSTART, "start" },
	{ VSTOP,  "stop"  },
	{ VSUSP,  "susp"  }
};

struct attr_info {
	int index;
	const char *name;
	
	// flag: should this attribute be
	// displayed even when it's disabled?
	bool important;
};
const struct attr_info LOOKUP_CFLAGS[] = {
	// Copy-Paste-Reformat from
	// termios-c_cflag.h
	{ CSTOPB, "cstopb", false }, // Send two stop bits, else one.
	{ CREAD,  "cread",  false }, // Enable receiver.
	{ PARENB, "parenb", false }, // Parity enable.
	{ PARODD, "parodd", true  }, // if negative, even parity
	{ HUPCL,  "hupcl",  false }, // Hang up on last close.
	{ CLOCAL, "clocal", false }  // Ignore modem status lines.
};
const struct attr_info LOOKUP_IFLAGS[] = {
	// Copy-Paste-Reformat from
	// termios-c_iflag.h
	{ IGNBRK,  "ignbrk",  false }, // Ignore break condition.
	{ BRKINT,  "brkint",  true  }, // Signal interrupt on break.
	{ IGNPAR,  "ignpar",  false }, // Ignore characters with parity errors.
	{ PARMRK,  "parmrk",  false }, // Mark parity and framing errors.
	{ INPCK,   "inpck",   true  }, // Enable input parity check.
	{ ISTRIP,  "istrip",  false }, // Strip 8th bit off characters.
	{ INLCR,   "inlcr",   false }, // Map NL to CR on input.
	{ IGNCR,   "igncr",   false }, // Ignore CR.
	{ ICRNL,   "icrnl",   true  }, // Map CR to NL on input.
	{ IUCLC,   "iuclc",   false }, // Map uppercase characters to lowercase on input (not in POSIX).
	{ IXON,    "ixon",    false }, // Enable start/stop output control.
	{ IXANY,   "ixany",   true  }, // Enable any character to restart output.
	{ IXOFF,   "ixoff",   false }, // Enable start/stop input control.
	{ IMAXBEL, "imaxbel", false }, // Ring bell when input queue is full (not in POSIX).
	{ IUTF8,   "iutf8",   false }  // Input is UTF8 (not in POSIX).
};
const struct attr_info LOOKUP_OFLAGS[] = {
	// Copy-Paste-Reformat from
	// termios-c_oflag.h
	{ OPOST,  "opost",  false }, // Post-process output.
	{ OLCUC,  "olcuc",  false }, // Map lowercase characters to uppercase on output. (not in POSIX).
	{ ONLCR,  "onlcr",  true  }, // Map NL to CR-NL on output.
	{ OCRNL,  "ocrnl",  false }, // Map CR to NL on output.
	{ ONOCR,  "onocr",  false }, // No CR output at column 0.
	{ ONLRET, "onlret", false }, // NL performs CR function.
	{ OFILL,  "ofill",  false }, // Use fill characters for delay.
	{ OFDEL,  "ofdel",  false }  // Fill is DEL.
	// { TABDLY, "notabs", true  }  // Horizontal-tab delays.
	// I don't know any good way of doing this
	// without doing weird dang special cases :(
	// Plus, TABDLY, TAB0, and TAB3 aren't even defined over here.
	// And I'm running Linux Linux! (openSUSE Tumbleweed...)
};
const struct attr_info LOOKUP_LFLAGS[] = {
	// Copy-Paste-Reformat from
	// termios-c_lflag.h
	{ ISIG,   "isig"   , false }, // Enable signals.
	{ ICANON, "icanon" , false }, // Canonical input (erase and kill processing).
	{ ECHO,   "echo"   , true  }, // Enable echo.
	{ ECHOE,  "echoe"  , true  }, // Echo erase character as error-correcting backspace.
	{ ECHOK,  "echok"  , true  }, // Echo KILL.
	{ ECHONL, "echonl" , false }, // Echo NL.
	{ NOFLSH, "noflsh" , false }, // Disable flush after interrupt or quit.
	{ TOSTOP, "tostop" , false }, // Send SIGTTOU for background output.
	{ IEXTEN, "iexten" , true  }  // Enable implementation-defined input processing.
};

// Convert a control character into one that can be displayed.
char display_cc(cc_t cc) {
	// bleh https://en.wikipedia.org/wiki/Caret_notation
	return cc ^ 0x40;
}

// Parse a string like "^C" into a control character.
cc_t parse_cc(const char* v) {
	size_t l = strlen(v);
	if (l == 0 || l > 2) return _POSIX_VDISABLE;
	if (l == 2 && v[0] == '^') v = &v[1];
	return (*v) ^ 0x40;
}

// Print what key the specified control character
// is bound to, in the specified terminal.
void print_control_char(
	const struct termios* term,
	const struct cc_info* info
) {
	// only print control character if in valid range
	// as specified by the termios headers
	if (info->index < NCCS) {
		cc_t cc = term->c_cc[info->index];
		if (cc == _POSIX_VDISABLE) {
			printf("%s = <undef>", info->name);
		} else {
			char printable_cc = display_cc(cc);
			printf("%s = ^%c", info->name, printable_cc);
			// printf(" (%d; %d)", cc, printable_cc);
		}
	} else {
		printf("invalid (%d)", info->index);
	}
}

// Print the current status of all attributes in the bitset,
// looking at the attr_info table for attribute names.
void print_attr_info(
	tcflag_t bitset,
	const struct attr_info lookup[],
	size_t lookup_len,
	bool all
) {
	bool printed_one = false;
	for (size_t i = 0; i < lookup_len; i++) {
		const struct attr_info* info = &lookup[i];
		
		bool enabled = bitset & info->index;
		bool should_display = enabled || all || info->important;
		
		if (should_display) {
			// if we've already printed one attribute before,
			// print a separating space for readability.
			if (printed_one) putchar(' ');
			
			if (!enabled) putchar('-');
			printf("%s", info->name);
			
			// (and set the aforementioned "already printed once" flag)
			printed_one = true;
		}
	}
}

// Find first instance of "name" in the provided lookup table.
// Otherwise, returns NULL.
const struct attr_info* find_attr_info(
	const char* name,
	const struct attr_info lookup[],
	size_t lookup_len
) {
	for (size_t i = 0; i < lookup_len; i++)
		if (strcmp(name, lookup[i].name) == 0)
			return &lookup[i];
	return NULL;
}

// Find first instance of "name" in the provided lookup table.
// Otherwise, returns NULL.
// Again. (Beginning to miss generics a bit.)
const struct cc_info* find_cc_info(
	const char* name,
	const struct cc_info lookup[],
	size_t lookup_len
) {
	for (size_t i = 0; i < lookup_len; i++)
		if (strcmp(name, lookup[i].name) == 0)
			return &lookup[i];
	return NULL;
}

void print_terminal_info(const struct termios* term) {
	// (the `all` flag is unused, but if you want to see *every* flag
	//  in the lookup tables, you can set it to true here. otherwise,
	//  attributes without the "important" flag set will only be
	//  visible when they are set.)
	bool all = false;
	
	// Show terminal baud speed.
	
	speed_t baud = cfgetospeed(term);
	char *baud_str = get_baud_str(baud);
	if (baud_str != NULL) {
		printf("speed %s baud; ", baud_str);
	} else {
		fprintf(stderr, "unknown speed (%d)\n", baud);
	}
	
	// Show a few important attributes for now.
	print_attr_info(term->c_cflag, arrandsize(LOOKUP_CFLAGS), all); printf("\n");
	
	// Show current control character assignments.
	
	for (size_t i = 0; i < sizeofarr(LOOKUP_CONTROL_CHARS); i++) {
		if (i > 0) printf("; ");
		print_control_char(term, &LOOKUP_CONTROL_CHARS[i]);
	}
	printf("\n");
	
	// Show the rest of the important attributes.
	print_attr_info(term->c_iflag, arrandsize(LOOKUP_IFLAGS), all); printf("\n");
	print_attr_info(term->c_oflag, arrandsize(LOOKUP_OFLAGS), all); printf("\n");
	print_attr_info(term->c_lflag, arrandsize(LOOKUP_LFLAGS), all); printf("\n");
}

bool set_terminal_attr_by_name(struct termios* term, const char *name) {
	// Parse leading -. For fun, I've also added support
	// for a leading ~ to easily toggle attributes.
	enum bit_action action = BIT_ON;
	if (name[0] == '-') {
		action = BIT_OFF;
		name = &name[1];
	} else if (name[0] == '~') {
		action = BIT_INVERT;
		name = &name[1];
	}
	
	const struct attr_info* attr = NULL;
	tcflag_t* flag_set = NULL;
	// (this is so indirect out of convenience: now i don't have to copy
	//  and paste the `switch (action)` block under each successful path)
	
	// Find attribute info in one of the four lookup tables.
	if      ((attr = find_attr_info(name, arrandsize(LOOKUP_IFLAGS))) != NULL)
		flag_set = &(term->c_iflag);
	else if ((attr = find_attr_info(name, arrandsize(LOOKUP_OFLAGS))) != NULL)
		flag_set = &(term->c_oflag);
	else if ((attr = find_attr_info(name, arrandsize(LOOKUP_CFLAGS))) != NULL)
		flag_set = &(term->c_cflag);
	else if ((attr = find_attr_info(name, arrandsize(LOOKUP_LFLAGS))) != NULL)
		flag_set = &(term->c_lflag);
	else
		return false;
	
	switch (action) {
		case BIT_OFF:    *flag_set &= ~(attr->index); break;
		case BIT_ON:     *flag_set |= attr->index; break;
		case BIT_INVERT: *flag_set ^= attr->index; break;
		default: return false;
	}	
	return true;
}

int main(int argc, char *argv[]) {
	struct termios term;
	if (tcgetattr(STDIN_FILENO, &term) == -1) {
		perror("couldn't get terminal info");
		exit(EXIT_FAILURE);
	}
	
	if (argc <= 1) {
		print_terminal_info(&term);
	} else {
		// Pass the terminal info struct into
		// several functions that mutate it...
		
		for (int i = 1; i < argc; i++) {
			const char *name = argv[i];
			
			// See if it's a control character name...
			
			const struct cc_info* cc_found =
				find_cc_info(name, arrandsize(LOOKUP_CONTROL_CHARS));
			if (cc_found != NULL) {
				// If so, skip to and look at the next argument.
				i++;
				
				// Make sure to bounds-check!
				if (i >= argc) {
					fprintf(stderr, "incomplete cc assignment (%s)\n", name);
					exit(EXIT_FAILURE);
				}
				
				// I expect this to be either one or two characters in length,
				// and if it's two characters, the first must be a ^.
				const char *value = argv[i];
				
				cc_t cc = parse_cc(value);
				term.c_cc[cc_found->index] = cc;
				
				// Since it was successfully found to be a control character
				// and since it's been assigned,  uhhh that's it
				continue;
			}
			
			// Sorry, this unassuming single function call does so much!!!
			// It even parses out the first character to decide if it enables
			// or disables attributes. It's okay.
			bool attr_found = set_terminal_attr_by_name(&term, name);
			
			if (!attr_found) {
				fprintf(stderr, "unknown mode (%s)\n", name);
				exit(EXIT_FAILURE);
			}
		}
		
		// ...and write the resulting struct back into the terminal!
		tcsetattr(STDIN_FILENO, TCSADRAIN, &term);
	}
	
	return EXIT_SUCCESS;
}
```

## `OLCUC` demonstration

For fun, I added a non-standard attribute leading character. If you add a leading tilde, (like `~olcuc`) instead of masking the attribute bit, it'll toggle the bit using XOR.

Also, I included the argumentless output of `sttyl` afterwards.

Note two things about the output, compared to the output provided in the document:

- instead of displaying "`evenp`", I display "`-parodd`". This means the same thing, and is easier to print using the lookup tables I gathered.
- `tabs` is unfortunately completely absent. I tried to put it in the lookup tables, but it seems to be difficult to do without special cases just for it. Also, the larger issue is that my compiler doesn't actually think the `TAB0` / `TAB3` constants exist! (They were present in my system's `termios.h`, but they were behind some internal preprocessor `#if`s that I didn't want to touch.)

```text
sh-5.2$ ./sttyl olcuc
SH-5.2$ ./STTYL OLCUC
SH-5.2$ ./STTYL -OLCUC
sh-5.2$ ./sttyl -olcuc
sh-5.2$ ./sttyl ~olcuc
SH-5.2$ ./STTYL ~OLCUC
sh-5.2$ ./sttyl ~olcuc
SH-5.2$ ./STTYL -OLCUC
sh-5.2$ ./sttyl
speed 38400 baud; cread -parodd
intr = ^C; erase = ^?; kill = ^U; start = ^Q; stop = ^S; susp = ^Z
-brkint -inpck icrnl ixon -ixany iutf8
opost onlcr
isig icanon echo echoe echok iexten
```
