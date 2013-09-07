/* 
 * $Id: main.c,v 1.4 2007/01/10 13:11:30 aoyama Exp $
 * 
 * Copyright (c) 2006 Kenji AOYAMA <aoyama@nk-home.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/ioctl.h>
#include <sys/param.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dev/usb/usb.h>
#include <dev/usb/usbhid.h>

/* prototypes */
void usage(void);
int main(int, char *[]);
int check_device(char *);

#define USBRH_VENDOR  0x1774
#define USBRH_PRODUCT 0x1001
#define DEFAULT_DEVICE "/dev/uhid0"

extern char *__progname;
extern char *optarg;
extern int optind;
extern float calc_temp(unsigned int);
extern float calc_humid(unsigned int, float);

int vflag = 0;

int
main(int argc, char *argv[])
{
	struct usb_ctl_report ucr;
	int ch, fd, ret;
	float t, h;
	u_char buf[8];
	char devname[MAXPATHLEN];

	strlcpy(devname, DEFAULT_DEVICE, sizeof(devname));

	while ((ch = getopt(argc, argv, "f:v")) != -1) {
		switch(ch) {
		case 'f':
			strlcpy(devname, optarg, sizeof(devname));
			break;
		case 'v':
			vflag = 1;
			break;
		default:
			usage();
			/* NOTREACHED */
		}
	}
	argc -= optind;
	argv += optind;

	if (check_device(devname) != 1) {
		fprintf(stderr,	"%s: can not find USBRH device on %s\n",
			__progname, devname);
		exit(1);
	}

	if ((fd = open(devname, O_RDWR)) < 0) {
		perror(devname);
		exit(1);
	}

	/*
	 * XXX: If we issue SET_REPORT only once, read(2) sometimes
	 * blocks.  So we issue SET_REPORT twice, with 1 second interval,
	 * that seems OK.
	 */

	ucr.ucr_report = UHID_OUTPUT_REPORT;
	if (ioctl(fd, USB_SET_REPORT, &ucr) < 0) {
		perror("USB_SET_REPORT");
		exit(1);
	}

	sleep(1);

	ucr.ucr_report = UHID_OUTPUT_REPORT;
	if (ioctl(fd, USB_SET_REPORT, &ucr) < 0) {
		perror("USB_SET_REPORT");
		exit(1);
	}

	if ((ret = read(fd, buf, 7)) == -1) {
		fprintf(stderr, "read error\n");
		exit(1);
	}

	close(fd);

	/*
	 * Now we got 7 bytes data from the device.
	 *
	 * buf[0] Humidity raw data (high byte)
	 * buf[1] Humidity raw data (low byte)
	 * buf[2] Temperature raw data (high byte)
	 * buf[3] Temperature raw data (low byte)
	 * buf[4] ???
	 * buf[5] ???
	 * buf[6] ???
	 */

	/* Calculate the real values */
	t = calc_temp(buf[2] * 256 + buf[3]);
	h = calc_humid(buf[0] * 256 + buf[1], t);

	if (vflag) {
		printf("Temperature = %.2f C\n", t);
		printf("Humidity    = %.1f %%\n", h);
	} else
		printf("%.2f\t%.1f\n", t, h);

	return 0;
}

void
usage(void)
{
	fprintf(stderr, "usage: %s [-v] [-f device]\n", __progname);
	exit(1);
}

/*
 * Check USBRH vendor ID and product ID on specified device.
 * Return 1 if matches.
 */
int
check_device(char *devname)
{
	int fd;
	struct usb_device_info udi;

#if defined(__NetBSD__) || defined(__OpenBSD__)

	/* On NetBSD/OpenBSD, /dev/uhid? can accept USB_GET_DEVICEINFO ioctl. */

	if ((fd = open(devname, O_RDWR)) < 0) {
		perror(devname);
		exit(1);
	}
	
	if (ioctl(fd, USB_GET_DEVICEINFO, &udi) < 0) {
		perror("USB_GET_DEVICEINFO");
		exit(1);
	}

	close(fd);

	/* vendor and product ID check */
	if ((udi.udi_vendorNo == USBRH_VENDOR) &&
		(udi.udi_productNo == USBRH_PRODUCT)) {
		return 1;
	}

	return 0;

#else

	/* Otherwise, walk through from /dev/usb? devices. */

	int cnt, addr;
	char buf1[MAXPATHLEN], buf2[MAXPATHLEN];

	for (cnt = 0; cnt < 10; cnt++) {	/* XXX: is this enough? */
		snprintf(buf1, sizeof(buf1), "/dev/usb%d", cnt);
		fd = open(buf1, O_RDONLY);
		if (fd < 0)
			continue;	
		for (addr = 1; addr < USB_MAX_DEVICES; addr++) {
			udi.udi_addr = addr;
			if (ioctl(fd, USB_DEVICEINFO, &udi) < 0)
				continue;
			strlcpy(buf2, "/dev/", sizeof(buf2));
			strlcat(buf2, udi.udi_devnames[0], sizeof(buf2));
#ifdef DEBUG
			printf("%s Addr %d, Vendor 0x%04x, Product 0x%04x, "
				"%s, %s, %s, %s, %s\n", 
				buf1, addr, udi.udi_vendorNo, udi.udi_productNo,
				udi.udi_devnames[0], udi.udi_devnames[1],
				udi.udi_devnames[2], udi.udi_devnames[3],
				buf2);
#endif
			if ((strcmp(devname, buf2) == 0) &&
				(udi.udi_vendorNo == USBRH_VENDOR) &&
				(udi.udi_productNo == USBRH_PRODUCT)) {
				return 1;
			}
		}
		close(fd);
	}
	return 0;
#endif
} 
