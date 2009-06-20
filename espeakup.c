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
espeak_AUDIO_OUTPUT audio_mode;
t_espeak_callback *audio_callback = NULL;

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

void espeakup_sighandler(int sig)
{
	if (debug)
		printf("Caught signal %i\n", sig);

	/* clear the queue */
	queue_clear();

	/* shut down espeak and close the softsynth */
	espeak_Terminate();
	close_softsynth();

	if (!debug)
		unlink(pidPath);
	exit(0);
}

int main(int argc, char **argv)
{
	pthread_t queue_thread_id;
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

	/* open the softsynth. */
	if (!open_softsynth()) {
		perror("Unable to open the softsynth device");
		return 3;
	}

	/* register signal handler */
	signal(SIGINT, espeakup_sighandler);
	signal(SIGTERM, espeakup_sighandler);

	if (init_audio() < 0) {
		return 5;
	}

	if (!debug) {
		/* become a daemon */
		daemon(0, 1);

		/* write our pid file. */
		if (create_pid_file() < 0) {
			perror("Unable to create pid file");
			return 2;
		}
	}

	/* initialize espeak */
	espeak_Initialize(audio_mode, 0, NULL, 0);
	espeak_SetSynthCallback(audio_callback);

	/* Setup initial voice parameters */
	if (defaultVoice) {
		set_voice(&s, defaultVoice);
		free(defaultVoice);
		defaultVoice = NULL;
	}
	set_frequency(&s, defaultFrequency, ADJ_SET);
	set_pitch(&s, defaultPitch, ADJ_SET);
	set_rate(&s, defaultRate, ADJ_SET);
	set_volume(&s, defaultVolume, ADJ_SET);
	espeak_SetParameter(espeakCAPITALS, 0, 0);

	/* Spawn our queue-processing thread. */
	int err = pthread_create(&queue_thread_id, NULL, &queue_runner, &s);
	if (err != 0) {
		return 4;
	}

	/* run the main loop */
	main_loop(&s);

	return 0;
}
