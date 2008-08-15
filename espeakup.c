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

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "espeakup.h"

/* program version */
const char *Version = "0.1";

/* command line options */
const char *shortOptions = "dhv";

/* path to our pid file */
const char *pidPath = "/var/run/espeakup.pid";

int debug = 0;
int softFD;

int espeakup_is_running(void)
{
	int rc;
	FILE *pidFile;
	pid_t pid;

	rc = 0;
	pidFile = fopen(pidPath, "r");
	if (pidFile) {
		fscanf(pidFile, "%d", &pid);
		fclose(pidFile);
		if (! kill(pid, 0) || errno != ESRCH)
			rc = 1;
	}
	return rc;
}

int create_pid_file(void)
{
	FILE *pidFile;

	pidFile = fopen(pidPath, "w");
	if (! pidFile)
		return -1;

	fprintf(pidFile, "%d\n", getpid());
	fclose(pidFile);
	return 0;
}
 
void espeakup_sighandler(int sig)
{
	if (debug)
		printf("Caught signal %i\n", sig);
	
	/* shutdown espeak and close the softsynth */
	espeak_Terminate();
	close(softFD);

	if (! debug)
		unlink(pidPath);
	exit(0);
}

void show_help()
{
	printf("Usage:  espeakup [-d] [-h] [-v]\n\n");
	printf("-d\tDebug mode (stay in the foreground)\n");
	printf("-h\tShow this help\n");
	printf("-v\tDisplay the software version.\n");
	exit(0);
}

void show_version(void)
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

	while ((opt = getopt(argc, argv, shortOptions)) != -1) {
		switch(opt) {
		case 'd':
			debug = 1;
			break;
		case 'h':
			show_help();
			break;
		case 'v':
			show_version();
			break;
		default:
			show_help();
			break;
		}
	}
}

int main(int argc, char **argv)
{
	struct synth_t s = {
		.frequency = 5,
		.pitch = 5,
		.rate = 5,
		.voice = "default",
		.volume = 5,
	};

	/* process command line options */
	process_cli(argc, argv);

	if (! debug) {
		if (espeakup_is_running()) {
			printf("Espeakup is already running!\n");
			return 1;
		}

		/* become a daemon */
		daemon(0, 1);

		/* write our pid file. */
		if (create_pid_file() < 0) {
			perror("Unable to create pid file");
			return 2;
		}
	}

	/* open the softsynth. */
	softFD = open ("/dev/softsynth", O_RDWR | O_NONBLOCK);
	if (softFD < 0) {
		perror("Unable to open the softsynth device");
		return 3;
	}

	/* initialize espeak */
	espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL, 0);
	espeak_SetSynthCallback(SynthCallback);

	/* Setup initial voice parameters */
	set_voice(&s);
	set_frequency (&s);
	set_pitch (&s);
	set_rate (&s);
	set_volume(&s);

	/* register signal handler */
	signal(SIGINT, espeakup_sighandler);
	signal(SIGTERM, espeakup_sighandler);

	/* run the main loop */
	main_loop (&s);

	return 0;
}
