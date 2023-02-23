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

const char* BAUD_LIST[] = {
	"0", "50", "75", "110", "134", "150", "200", "300", "600",
	"1200", "1800", "2400", "4800", "9600", "19200", "38400",
	"57600", "115200", "230400", "460800", "500000", "576000",
	"921600", "1000000", "1152000", "1500000", "2000000",
};

int main(int argc, char *argv[]) {
	/* if (argc <= 1) {
		char *name = (argc > 0) ? argv[0] : "sttyl";
		fprintf(stderr, "Usage: %s some thing\n", name);
		exit(EXIT_FAILURE);
	} */
	
	struct termios terminalInfo;
	if (tcgetattr(STDIN_FILENO, &terminalInfo) == -1) {
		perror("oops");
		exit(EXIT_FAILURE);
	}
	
	// the returned value of this is yeah. the freakin numbers from constants
	// its an index into this..
	speed_t baud = cfgetospeed(&terminalInfo);
	printf("speed %s baud\n", BAUD_LIST[baud]);
	
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
