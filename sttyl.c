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

#define sizeofarr(arr) (sizeof(arr) / sizeof(*arr))

char *get_baud_str(speed_t baud) {
	switch (baud) {
		case B0:     return "0";     case B50:    return "50";
		case B75:    return "75";    case B110:   return "110";
		case B134:   return "134";   case B150:   return "150";
		case B200:   return "200";   case B300:   return "300";
		case B600:   return "600";   case B1200:  return "1200";
		case B1800:  return "1800";  case B2400:  return "2400";
		case B4800:  return "4800";  case B9600:  return "9600";
		case B19200: return "19200"; case B38400: return "38400";
	}
	return NULL;
}

void print_control_char(struct termios* term, char *msg, size_t index) {
	if (index < NCCS) {
		cc_t cc = term->c_cc[index];
		if (cc == _POSIX_VDISABLE) {
			printf("%s = <none>", msg);
		} else {
			// Make printable if alphabetical.
			char printable_cc = cc < 26
				? cc + 'A' - 1
				: '?'; //cc & 077;
			
			printf("%s = ^%c", msg, printable_cc);
			// printf(" (%d; %d)", cc, printable_cc);
		}
	} else {
		fprintf(stderr, "unknown control char (%ld)", index);
	}
}

int main(int argc, char *argv[]) {
	struct termios terminalInfo;
	if (tcgetattr(STDIN_FILENO, &terminalInfo) == -1) {
		perror("couldn't get terminal info");
		exit(EXIT_FAILURE);
	}
	
	// Show terminal baud speed.
	
	speed_t baud = cfgetospeed(&terminalInfo);
	char *baud_str = get_baud_str(baud);
	if (baud_str != NULL) {
		printf("speed %s baud\n", baud_str);
	} else {
		fprintf(stderr, "unknown speed (%d)\n", baud);
	}
	
	// Show control characters.
	
	struct {
		size_t index;
		char *name;
	} listedCharacters[] = {
		VINTR,  "intr",
		VERASE, "erase",
		VKILL,  "kill",
		VSTART, "start",
		VSTOP,  "stop",
	};
	
	for (size_t i = 0; i < sizeofarr(listedCharacters); i++) {
		if (i > 0) printf("; ");
		
		print_control_char(
			&terminalInfo,
			listedCharacters[i].name,
			listedCharacters[i].index
		);
	}
	printf("\n");
	
	return EXIT_SUCCESS;
}

/*
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

/*
terminalInfo = {
	c_iflag = 0x2d02, // input modes
	c_oflag = 0x0005, // output modes
	c_cflag = 0x04bf, // control modes
	c_lflag = 0x8a3b, // local modes
	c_cc = { // special characters
		0x03, 0x1c, 0x7f, 0x15, 0x04, 0x00, 0x01,
		0x00, 0x11, 0x13, 0x1a, 0xff, 0x12, 0x0f,
		0x17, 0x16, 0xff, 0x00 <repeats 15 times>
	},
	c_line   = 0x0000,
	c_ispeed = 0x000f,
	c_ospeed = 0x000f
}
*/
