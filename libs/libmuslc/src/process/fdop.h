/* @LICENSE(MUSLC_MIT) */

#define FDOP_CLOSE 1
#define FDOP_DUP2 2
#define FDOP_OPEN 3

struct fdop {
	struct fdop *next, *prev;
	int cmd, fd, newfd, oflag;
	mode_t mode;
	char path[];
};
