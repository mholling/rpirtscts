/*
    A command-line utility for enabling hardware flow control on the
    Raspberry Pi serial port.

    Copyright (C) 2013 Matthew Hollingworth.

    40 pin header support for newer Raspberry Pis 
    Copyright (C) 2016 Brendan Traw.

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

#define GPIO_OFFSET (0x200000)
#define BLOCK_SIZE (4*1024)
#define GFPSEL3 (3)
#define GPIO3031mask 0x0000003f /* GPIO 30 for CTS0 and 31 for RTS0 */
#define GFPSEL1 (1)
#define GPIO1617mask 0x00fc0000 /* GPIO 16 for CTS0 and 17 for RTS0 */

#define GPIO_header_26 0x00
#define GPIO_header_40 0x01

#define VERSION "1.5"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

unsigned gpio_base()
{ /* adapted from bcm_host.c */
	unsigned address = ~0;
	FILE *fp = fopen("/proc/device-tree/soc/ranges", "rb");
	if (fp) {
		unsigned char buf[4];
		fseek(fp, 4, SEEK_SET);
		if (fread(buf, 1, sizeof buf, fp) == sizeof buf)
			address = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3] << 0;
		fclose(fp);
	}
	return (address == ~0 ? 0x20000000 : address) + GPIO_OFFSET;
}

int rpi_version() {
	int result = -1;
	char string[256];
	FILE *fp = fopen("/proc/cpuinfo", "r");
	if (fp) {
		while (fscanf(fp, "%255s", string) == 1)
			if (strcmp(string, "Revision") == 0)
				break;
		while (fscanf(fp, "%255s", string) == 1)
			if (sscanf(string, "%x", &result) == 1)
				break;
		fclose(fp);
	}
	if (result < 0) {
		fprintf(stderr, "can't parse /proc/cpuinfo\n");
		exit(EXIT_FAILURE);
	} else
		result &= ~(1 << 24 | 1 << 25); // clear warranty bits
	return result;
}

int rpi_gpio_header_type() {
	switch (rpi_version()) { /* Adapted from http://elinux.org/RPi_HardwareHistory */
	case 0x000002: // Model B Rev 1.0
	case 0x000003: // Model B Rev 1.0+
	case 0x000004: // Model B Rev 2.0
	case 0x000005: // Model B Rev 2.0
	case 0x000006: // Model B Rev 2.0
	case 0x000007: // Model A
	case 0x000008: // Model A
	case 0x000009: // Model A
	case 0x00000d: // Model B Rev 2.0
	case 0x00000e: // Model B Rev 2.0
	case 0x00000f: // Model B Rev 2.0
		printf("26-pin GPIO header detected\n");
		return GPIO_header_26;
	case 0x000011: // Compute Module 1
	case 0x000014: // Compute Module 1
	case 0xa020a0: // Compute Module 3
		fprintf(stderr, "compute module not supported\n");
		exit(EXIT_FAILURE);
	case 0x000010: // Model B+ Rev 1.0
	case 0x000012: // Model A+ Rev 1.1
	case 0x000013: // Model B+ Rev 1.2
	case 0x000015: // Model A+ Rev 1.1
	case 0x900021: // Model A+ Rev 1.1
	case 0x900032: // Model B+ Rev 1.2
	case 0x900092: // Pi Zero Rev 1.2
	case 0x900093: // Pi Zero Rev 1.3
	case 0x9000c1: // Pi Zero W
	case 0x920093: // Pi Zero Rev 1.3
	case 0xa01040: // Pi 2 Model B Rev 1.0
	case 0xa01041: // Pi 2 Model B Rev 1.1
	case 0xa02082: // Pi 3 Model B Rev 1.2
	case 0xa21041: // Pi 2 Model B Rev 1.1
	case 0xa22042: // Pi 2 Model B Rev 1.2
	case 0xa22082: // Pi 3 Model B Rev 1.2
	case 0xa32082: // Pi 3 Model B Rev 1.2
		printf("40-pin GPIO header detected\n");
		return GPIO_header_40;
	default:
		printf("assuming 40-pin GPIO header\n");
		return GPIO_header_40;
	}
}


void set_rts_cts(int enable) {
	int gfpsel, gpiomask;
	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	if (fd < 0) {
		fprintf(stderr, "can't open /dev/mem (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	void *gpio_map = mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, gpio_base());
	close(fd);
	if (gpio_map == MAP_FAILED) {
		fprintf(stderr, "mmap error (%s)\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	volatile unsigned *gpio = (volatile unsigned *)gpio_map;

	if (rpi_gpio_header_type() == GPIO_header_40) { /* newer 40 pin GPIO header */
		gfpsel = GFPSEL1;
		gpiomask = GPIO1617mask;
		enable ? printf("Enabling ") : printf("Disabling ");
		printf("CTS0 and RTS0 on GPIOs 16 and 17\n");
	}
	else { /* 26 pin GPIO header */
		gfpsel = GFPSEL3;
		gpiomask = GPIO3031mask;
		enable ? printf("Enabling ") : printf("Disabling ");
		printf("CTS0 and RTS0 on GPIOs 30 and 31\n");
	}
	
	enable ? (gpio[gfpsel] |= gpiomask) : (gpio[gfpsel] &= ~gpiomask);
}

void print_usage() {
	printf( \
	"Version: " VERSION "\n" \
	"Usage: rpirtscts on|off\n" \
	"Enable or disable hardware flow control pins on ttyAMA0.\n" \
	"\nFor 26 pin GPIO header boards:\n"    \
	"P5 header pins remap as follows:\n"	\
	"    P5-05 (GPIO30) -> CTS (input)\n" \
	"    P5-06 (GPIO31) -> RTS (output)\n" \
	"\nFor 40 pin GPIO header boards:\n"    \
	"    P1-36 (GPIO16) -> CTS (input)\n" \
	"    P1-11 (GPIO17) -> RTS (output)\n" \
	"\nYou may also need to enable flow control in the driver:\n" \
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
