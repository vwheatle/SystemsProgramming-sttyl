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

// actually i don't like this macro
#define setbit(i, mask, value) do { i = ((i) & ~(mask)) | ((mask) * (value)); } while (0)

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
	{ VSTOP,  "stop"  }
};

struct attr_info {
	int index;
	const char *name;
	
	// should this be printed
	// even when it's disabled?
	bool important;
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
	{ ONLCR,  "onlcr",  false }, // Map NL to CR-NL on output.
	{ OCRNL,  "ocrnl",  false }, // Map CR to NL on output.
	{ ONOCR,  "onocr",  false }, // No CR output at column 0.
	{ ONLRET, "onlret", false }, // NL performs CR function.
	{ OFILL,  "ofill",  false }, // Use fill characters for delay.
	{ OFDEL,  "ofdel",  false }  // Fill is DEL.
};
const struct attr_info LOOKUP_LFLAGS[] = {
	// Copy-Paste-Reformat from
	// termios-c_lflag.h
	{ ISIG,   "isig"   , false }, // Enable signals.
	{ ICANON, "icanon" , false }, // Canonical input (erase and kill processing).
	{ ECHO,   "echo"   , true  }, // Enable echo.
	{ ECHOE,  "echoe"  , true  }, // Echo erase character as error-correcting backspace.
	{ ECHOK,  "echok"  , false }, // Echo KILL.
	{ ECHONL, "echonl" , false }, // Echo NL.
	{ NOFLSH, "noflsh" , false }, // Disable flush after interrupt or quit.
	{ TOSTOP, "tostop" , false }, // Send SIGTTOU for background output.
	{ IEXTEN, "iexten" , false }  // Enable implementation-defined input processing.
};

void print_control_char(
	const struct termios* term,
	const struct cc_info* info
) {
	if (info->index < NCCS) {
		cc_t cc = term->c_cc[info->index];
		if (cc == _POSIX_VDISABLE) {
			printf("%s = <undef>", info->name);
		} else {
			// Make printable if alphabetical.
			char printable_cc = cc < 26
				? cc + 'A' - 1
				: '?'; //cc & 077;
			
			printf("%s = ^%c", info->name, printable_cc);
			// printf(" (%d; %d)", cc, printable_cc);
		}
	} else {
		printf("invalid (%d)", info->index);
	}
}

void print_attr_info(
	const tcflag_t* bitset,
	const struct attr_info lookup[],
	size_t lookup_len,
	bool all
) {
	bool printed_one = false;
	for (size_t i = 0; i < lookup_len; i++) {
		const struct attr_info* info = &lookup[i];
		bool enabled = (*bitset) & info->index;
		bool should_display = enabled || all || info->important;
		if (should_display) {
			if (printed_one) putchar(' ');
			if (!enabled) putchar('-');
			printf("%s", info->name);
			printed_one = true;
		}
	}
}

const struct attr_info* find_attr_by_name(
	const char* name,
	const struct attr_info lookup[],
	size_t lookup_len
) {
	for (size_t i = 0; i < lookup_len; i++)
		if (strcmp(name, lookup[i].name) == 0)
			return &lookup[i];
	return NULL;
}

const struct cc_info* find_cc_by_name(
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
	// Show terminal baud speed.
	
	speed_t baud = cfgetospeed(term);
	char *baud_str = get_baud_str(baud);
	if (baud_str != NULL) {
		printf("speed %s baud", baud_str);
	} else {
		fprintf(stderr, "unknown speed (%d)", baud);
	}
	printf("\n");
	
	// Show current control character assignments.
	
	for (size_t i = 0; i < sizeofarr(LOOKUP_CONTROL_CHARS); i++) {
		if (i > 0) printf("; ");
		print_control_char(term, &LOOKUP_CONTROL_CHARS[i]);
	}
	printf("\n");
	
	// Show important flags
	
	bool all = false;
	print_attr_info(&(term->c_iflag), arrandsize(LOOKUP_IFLAGS), all); printf("\n");
	print_attr_info(&(term->c_oflag), arrandsize(LOOKUP_OFLAGS), all); printf("\n");
	print_attr_info(&(term->c_lflag), arrandsize(LOOKUP_LFLAGS), all); printf("\n");
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
			
			const struct cc_info* cc_found = find_cc_by_name(name, arrandsize(LOOKUP_CONTROL_CHARS));
			if (cc_found != NULL) {
				// i++;
				// and then parse the control character
				// which is either ^c or c on its own
				continue;
			}
			
			bool set_bit_to = true;
			if (name[0] == '-') {
				set_bit_to = false;
				name = &name[1];
			}
			
			const struct attr_info* found;
			if ((found = find_attr_by_name(name, arrandsize(LOOKUP_IFLAGS))) != NULL) {
				setbit(term.c_iflag, found->index, set_bit_to);
				continue;
			}
			if ((found = find_attr_by_name(name, arrandsize(LOOKUP_OFLAGS))) != NULL) {
				setbit(term.c_oflag, found->index, set_bit_to);
				continue;
			}
			if ((found = find_attr_by_name(name, arrandsize(LOOKUP_LFLAGS))) != NULL) {
				setbit(term.c_lflag, found->index, set_bit_to);
				continue;
			}
			
			fprintf(stderr, "unknown mode (%s)", argv[i]);
			exit(EXIT_FAILURE);
		}
		
		// ...and write the resulting struct back into the terminal!
		tcsetattr(STDIN_FILENO, TCSADRAIN, &term);
	}
	
	return EXIT_SUCCESS;
}

/* example stty output
$ stty -a
speed 38400 baud; rows 24; columns 80; line = 0;
intr = ^C; quit = ^\; erase = ^?; kill = ^U; eof = ^D; eol = <undef>;
eol2 = <undef>; swtch = <undef>; start = ^Q; stop = ^S; susp = ^Z; rprnt = ^R;
werase = ^W; lnext = ^V; discard = ^O; min = 1; time = 0;
-parenb -parodd -cmspar cs8 -hupcl -cstopb cread -clocal -crtscts
-ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr icrnl ixon -ixoff
-iuclc -ixany -imaxbel iutf8
opost -olcuc -ocrnl onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0
isig icanon iexten echo echoe echok -echonl -noflsh -xcase -tostop -echoprt
echoctl echoke -flusho -extproc
*/
