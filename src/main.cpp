
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <stdint.h>
typedef uint16_t U16;
#define BUF_LEN 512

struct options {
	const char    *opt;
	unsigned char val;
};


static struct options kmod[] = {
	{.opt = "--left-ctrl",		.val = 0x01},
	{.opt = "--right-ctrl",		.val = 0x10},
	{.opt = "--left-shift",		.val = 0x02},
	{.opt = "--right-shift",	.val = 0x20},
	{.opt = "--left-alt",		.val = 0x04},
	{.opt = "--right-alt",		.val = 0x40},
	{.opt = "--left-meta",		.val = 0x08},
	{.opt = "--right-meta",		.val = 0x80},
	{.opt = NULL}
};

static struct options kval[] = {
	{.opt = "--return",	.val = 0x28},
	{.opt = "--esc",	.val = 0x29},
	{.opt = "--bckspc",	.val = 0x2a},
	{.opt = "--tab",	.val = 0x2b},
	{.opt = "--spacebar",	.val = 0x2c},
	{.opt = "--caps-lock",	.val = 0x39},
	{.opt = "--f1",		.val = 0x3a},
	{.opt = "--f2",		.val = 0x3b},
	{.opt = "--f3",		.val = 0x3c},
	{.opt = "--f4",		.val = 0x3d},
	{.opt = "--f5",		.val = 0x3e},
	{.opt = "--f6",		.val = 0x3f},
	{.opt = "--f7",		.val = 0x40},
	{.opt = "--f8",		.val = 0x41},
	{.opt = "--f9",		.val = 0x42},
	{.opt = "--f10",	.val = 0x43},
	{.opt = "--f11",	.val = 0x44},
	{.opt = "--f12",	.val = 0x45},
	{.opt = "--insert",	.val = 0x49},
	{.opt = "--home",	.val = 0x4a},
	{.opt = "--pageup",	.val = 0x4b},
	{.opt = "--del",	.val = 0x4c},
	{.opt = "--end",	.val = 0x4d},
	{.opt = "--pagedown",	.val = 0x4e},
	{.opt = "--right",	.val = 0x4f},
	{.opt = "--left",	.val = 0x50},
	{.opt = "--down",	.val = 0x51},
	{.opt = "--kp-enter",	.val = 0x58},
	{.opt = "--up",		.val = 0x52},
	{.opt = "--num-lock",	.val = 0x53},
	{.opt = NULL}
};

// Eventually Expand this to all of ascii!

struct ascii_map {
	const char   * opt;
	U16            val;
};
static struct ascii_map ascii_val[] = {
	{.opt = " ",	.val =   0x2c},
	{.opt = "!",	.val = 0x021e},
	{.opt = "\"",	.val = 0x0234},
	{.opt = "#",	.val = 0x0220},
	{.opt = "$",	.val = 0x0221},
	{.opt = "%",	.val = 0x0222},
	{.opt = "&",	.val = 0x0224},
	{.opt = "'",	.val =   0x34},
	{.opt = "(",	.val = 0x0226},
	{.opt = ")",	.val = 0x0227},
	{.opt = "*",	.val = 0x0225},
	{.opt = "+",	.val = 0x022e},
	{.opt = ",",	.val =   0x36},
	{.opt = "-",	.val =   0x2d},
	{.opt = ".",	.val =   0x37},
	{.opt = "/",	.val =   0x38},

	{.opt = "0",	.val =   0x27},
	{.opt = "1",	.val =   0x1e},
	{.opt = "2",	.val =   0x1f},
	{.opt = "3",	.val =   0x20},
	{.opt = "4",	.val =   0x21},
	{.opt = "5",	.val =   0x22},
	{.opt = "6",	.val =   0x23},
	{.opt = "7",	.val =   0x24},
	{.opt = "8",	.val =   0x25},
	{.opt = "9",	.val =   0x26},
	{.opt = ":",	.val = 0x0233},
	{.opt = ";",	.val =   0x33},
	{.opt = "<",	.val = 0x0236},
	{.opt = "=",	.val =   0x2e},
	{.opt = ">",	.val = 0x0237},
	{.opt = "?",	.val = 0x0238},
};

void
keyboard_gen_report(char report[8], char * byte)
{
    char c = byte[0];
	if (islower(c))
    {
		report[2] = (c - ('a' - 0x04));
	}
    else
	if (isupper(c))
    {
        c = tolower(c);
		report[2] = (c - ('a' - 0x04));
		report[0] = 2;  // Shift
	}
    else
	if (isdigit(c))
    {
		report[2] = ascii_val[(c - 32)].val;
	}
    else
	if (c == '\n')
    {
		report[2] = 0x28;
	}
    else
	if (c == '\t')
    {
		report[2] = 0x2b;
	}
    else
	if ((c >= ' ') && (c <= '/'))
    {
		report[2] = (0x00ff & ascii_val[(c - 32)].val) >> 0;
		report[0] = (0xff00 & ascii_val[(c - 32)].val) >> 8;
	}
}

void
check_reply(int fd)
{
	int i;
	int cmd_len;
        char buf[BUF_LEN];

	cmd_len = read(fd, buf, BUF_LEN - 1);
	if (cmd_len != -1)
	{
		printf("recv report:");
		for (i = 0; i < cmd_len; i++)
			printf(" %02x", buf[i]);
		printf("\n");
	}
	else
	{
		perror("read");
		exit(5);
	}
}

int main(int argc, const char *argv[])
{
	const char *filename = NULL;
	int fd = 0;
	char report[8];
	char null_report[8];
	int retval, i;
	fd_set rfds;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s devname 'string'\n",
			argv[0]);
		return 1;
	}

	filename = argv[1];

	if ((fd = open(filename, O_RDWR, 0666)) == -1) {
		fprintf(stderr, "open: %d\n", errno);
		perror(filename);
		return 3;
	}

	/*
        if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
		perror("fcntl");
		return 3;
	}
	*/

	//print_options(argv[2][0]);

        char * p = (char *)argv[2];

	memset(null_report, 0x0, sizeof(null_report));

        // Send Left-Shift
        // this is just to wake-up the screen saver
        // This is a one time thing and abuses null_report this one time
        null_report[0] = 2;
		if (write(fd, null_report, 8) != 8) {
			perror(filename);
			return 5;
		}
        usleep(100000); // delay 0.1 seconds
	memset(null_report, 0x0, sizeof(null_report));
        // Send Key-Up
		if (write(fd, null_report, 8) != 8) {
			perror(filename);
			return 5;
		}
        usleep(100000); // delay 0.1 seconds

        while (1)
        {
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		struct timeval poll = { 0,0 };

		retval = select(fd + 1, &rfds, NULL, NULL, &poll);
		if (retval == -1 && errno == EINTR)
			continue;
		if (retval < 0) {
			perror("select()");
			return 4;
		}


		if (FD_ISSET(fd, &rfds))
			check_reply(fd);

                if (*p == 0)
        		break;

		memset(report, 0x0, sizeof(report));

		keyboard_gen_report(report, p);

        // Send Key Down
		if (write(fd, report, 8) != 8) {
			perror(filename);
			return 5;
		}

        // Send Key Up - null_report
		if (write(fd, null_report, 8) != 8) {
			perror(filename);
			return 5;
		}
		p++;
	}

	close(fd);
	return 0;
}
