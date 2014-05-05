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
#include <string.h>

#include "espeakup.h"

/* program version */
extern const char *Version;

/* default voice */
extern char *defaultVoice;

/* default range */
extern int defaultRange;

/* rate control variables */
extern int rateMultiplier;
extern int rateOffset;

/* command line options */
const char *shortOptions = "dhV:m:o:r:v";
const struct option longOptions[] = {
	{"default-voice", required_argument, NULL, 'V'},
	{"rate-multiplier", required_argument, NULL, 'm'},
	{"rate-offset", required_argument, NULL, 'o'},
	{"range", required_argument, NULL, 'r'},
	{"debug", no_argument, NULL, 'd'},
	{"help", no_argument, NULL, 'h'},
	{"version", no_argument, NULL, 'v'},
	{0, 0, 0, 0}
};

static void show_help()
{
	printf("Usage: espeakup [options]\n\n");
	printf("Options are as follows:\n");
	printf("  --default-voice=voice, -V voice\tSet default voice.\n");
	printf("  --rate-multiplier=integer, -m integer\tSet rate multiplier.\n");
	printf("  --rate-offset=integer, -o integer\tSet rate offset.\n");
	printf("  --range=integer, -r integer\tSet range (between 0 and 100).\n");
	printf("  --debug, -d\t\t\t\tDebug mode (stay in the foreground).\n");
	printf("  --help, -h\t\t\t\tShow this help.\n");
	printf("  --version, -v\t\t\t\tDisplay the software version.\n");
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
		case 'V':
			defaultVoice = strdup(optarg);
			break;
		case 'm':
			rateMultiplier = atoi(optarg);
			break;
			case 'o':
			rateOffset = atoi(optarg);
			break;
		case 'r':
			defaultRange = atoi(optarg);
			break;
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
