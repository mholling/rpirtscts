/*
    A command-line utility for enabling hardware flow control on the
    Raspberry Pi serial port.
    
    Copyright (C) 2013 Matthew Hollingworth.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define GPIO_BASE (0x20200000)
#define BLOCK_SIZE (4*1024)
#define GFPSEL3 (3)

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void set_rts_cts(int enable) {
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "can't open /dev/mem (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	void *gpio_map = mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, GPIO_BASE);
	close(fd);
	if (gpio_map == MAP_FAILED) {
		fprintf(stderr, "mmap error (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	volatile unsigned *gpio = (volatile unsigned *)gpio_map;
	enable ? (gpio[GFPSEL3] |= 0x0000003F) : (gpio[GFPSEL3] &= ~0x0000003F);
}

void print_usage() {
	printf( \
	"Usage: rpirtscts on|off\n" \
	"Enable or disable hardware flow control pins on ttyAMA0.\n" \
	"P5 header pins remap as follows:\n" \
	"    P5-05 -> CTS (input)\n" \
	"    P5-06 -> RTS (output)\n" \
	"You may also need to enable flow control in the driver:\n" \
	"    stty -F /dev/ttyAMA0 crtscts\n" \
	);
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		print_usage();
	} else {
		int  enable = strcmp(argv[1],  "on") == 0;
		int disable = strcmp(argv[1], "off") == 0;
		enable || disable ? set_rts_cts(enable) : print_usage();
	}
	return EXIT_SUCCESS;
}
