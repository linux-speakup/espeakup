/*
 *  espeakup - interface which allows speakup to use espeak
 *
 *  Copyright (C) 2008 William Hubbs
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "espeakup.h"

/* program version */
const char *Version = "0.2";

/* command line options */
const char *shortOptions = "dhv";
const struct option longOptions[] = {
	{"debug", no_argument, NULL, 'd'},
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'v'},
	{0, 0, 0, 0}
};

static void show_help()
{
	printf("Usage: espeakup [options]\n\n");
	printf("Options are as follows:\n");
	printf("  --debug, -d\tDebug mode (stay in the foreground)\n");
	printf("  --help, -h\tShow this help\n");
	printf("  --version, -v\tDisplay the software version.\n");
	exit(0);
}

static void show_version(void)
{
	printf("espeakup %s\n", Version);
	printf("Copyright (C) 2008 William Hubbs\n");
	printf("License GPLv3+: GNU GPL version 3 or later\n");
	printf("You are free to change and redistribute this software.\n");
	exit(0);
}

void process_cli(int argc, char **argv)
{
	int opt;

	do {
		opt = getopt_long(argc, argv, shortOptions, longOptions, NULL);
		switch (opt) {
		case 'd':
			debug = 1;
			break;
		case 'h':
			show_help();
			break;
		case 'v':
			show_version();
			break;
		case -1:
			break;
		default:
			show_help();
			break;
		}
	} while (opt != -1);
}
