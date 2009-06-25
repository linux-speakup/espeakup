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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "espeakup.h"

/* program version */
const char *Version = "0.71";

/* path to our pid file */
const char *pidPath = "/var/run/espeakup.pid";

/* default voice settings */
const int defaultFrequency = 5;
const int defaultPitch = 5;
const int defaultRate = 5;
const int defaultVolume = 5;

char *defaultVoice = NULL;
int debug = 0;

volatile int stopped = 0;
volatile int should_run = 1;
espeak_AUDIO_OUTPUT audio_mode;

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
		if (!kill(pid, 0) || errno != ESRCH)
			rc = 1;
	}
	return rc;
}

int create_pid_file(void)
{
	FILE *pidFile;

	pidFile = fopen(pidPath, "w");
	if (!pidFile)
		return -1;

	fprintf(pidFile, "%d\n", getpid());
	fclose(pidFile);
	return 0;
}

int main(int argc, char **argv)
{
	sigset_t sigset;
	int rate;
	int err;
	pthread_t signal_thread_id;
	pthread_t queue_thread_id;
	pthread_t softsynth_thread_id;
	struct synth_t s = {
		.voice = "",
	};

	/* process command line options */
	process_cli(argc, argv);

	/* Is the espeakup daemon running? */
	if (espeakup_is_running()) {
		printf("Espeakup is already running!\n");
		return 1;
	}

/*
 * If we are not in debug mode, become a daemon and store the pid.
 */
	if (!debug) {
		daemon(0, 1);
		if (create_pid_file() < 0) {
			perror("Unable to create pid file");
			return 2;
		}
	}

	/* create the signal processing thread here. */
	err = pthread_create(&signal_thread_id, NULL, &signal_thread, NULL);
	if (err != 0) {
		return 4;
	}

	/*
	 * Set up the signal mask which will be the default for all threads.
	 * We are handling sigint and sigterm, so block them.
	 */
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	sigaddset(&sigset, SIGTERM);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	/* Spawn our queue-processing thread. */
	err = pthread_create(&queue_thread_id, NULL, &queue_runner, &s);
	if (err != 0) {
		return 4;
	}

	/* Spawn our softsynth thread. */
	err = pthread_create(&softsynth_thread_id, NULL, &softsynth_thread, &s);
	if (err != 0) {
		return 4;
	}

	return 0;
}
