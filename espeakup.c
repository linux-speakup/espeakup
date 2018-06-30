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
#include <fcntl.h>
#include <sys/file.h>

#include "espeakup.h"

/* path to our pid file */
char *pidPath = "/var/run/espeakup.pid";

int debug = 0;
enum espeakup_mode_t espeakup_mode = ESPEAKUP_MODE_SPEAKUP;
struct queue_t *synth_queue = NULL;

int self_pipe_fds[2];
volatile int should_run = 1;
espeak_AUDIO_OUTPUT audio_mode;

pthread_cond_t runner_awake = PTHREAD_COND_INITIALIZER;
pthread_cond_t stop_acknowledged = PTHREAD_COND_INITIALIZER;
pthread_mutex_t queue_guard = PTHREAD_MUTEX_INITIALIZER;

int espeakup_start_daemon(void)
{
	int fds[2];
	pid_t pid;
	char c;

	if (pipe(fds) < 0) {
		perror("pipe");
		exit(1);
	}
	pid = fork();

	if (pid < 0) {
		perror("fork");
		exit(1);
	}
	if (pid) {
		/* Parent, just wait for daemon */
		if (read(fds[0], &c, 1) < 0) {
			printf("Espeakup is already running!\n");
			exit(1);
		}
		exit(c);
	}

	/* Child, create new session */
	setsid();
	pid = fork();
	if (pid)
		/* Intermediate child, just exit */
		exit(0);

	/* Child */
	if (chdir("/") < 0) {
		c = 1;
		(void)write(fds[1], &c, 1);
		exit(1);
	}
	return fds[1];
}

int espeakup_is_running(void)
{
	int pidFile;
	int n;
	char s[16];
	pid_t pid;

	pidFile = open(pidPath, O_RDWR | O_CREAT, 0666);
	if (pidFile < 0) {
		printf("Can not work with the pid file %s: %s\n", pidPath,
		       strerror(errno));
		return -1;
	}

	if (flock(pidFile, LOCK_EX) < 0) {
		printf("Can not lock the pid file %s: %s\n", pidPath,
		       strerror(errno));
		goto error;
	}
	n = read(pidFile, s, sizeof(s) - 1);
	if (n < 0) {
		printf("Can not read the pid file %s: %s\n", pidPath,
		       strerror(errno));
		goto error;
	}
	s[n] = 0;
	n = sscanf(s, "%d", &pid);
	if (n == 1 && (!kill(pid, 0) || errno != ESRCH)) {
		/* Already running */
		close(pidFile);
		return 1;
	}
	if (ftruncate(pidFile, 0) < 0) {
		printf("Could not truncate the pid file %s: %s\n", pidPath,
		       strerror(errno));
		goto error;
	}
	lseek(pidFile, 0, SEEK_SET);
	n = snprintf(s, sizeof(s), "%d", getpid());
	if (write(pidFile, s, n) < 0) {
		printf("Could not write to the pid file %s: %s\n", pidPath,
		       strerror(errno));
		goto error;
	}
	close(pidFile);
	return 0;

error:
	close(pidFile);
	return -1;
}

int main(int argc, char **argv)
{
	int fd, devnull;
	char ret = 0;
	sigset_t sigset;
	int err;
	pthread_t signal_thread_id;
	pthread_t espeak_thread_id;
	pthread_t softsynth_thread_id;
	struct synth_t s = {
		.voice = "",
	};
	synth_queue = new_queue();

	if (!synth_queue) {
		fprintf(stderr, "Unable to allocate memory.\n");
		return 2;
	}

	/* set up the pipe used to wake the espeak thread */
	if (pipe(self_pipe_fds) < 0) {
		perror("Unable to create pipe");
		return 5;
	}

	/* process command line options */
	process_cli(argc, argv);

	if (!debug && espeakup_mode == ESPEAKUP_MODE_SPEAKUP) {
		fd = espeakup_start_daemon();

		if (espeakup_is_running()) {
			printf("Espeakup is already running!\n");
			ret = 1;
			goto out;
		}

		devnull = open("/dev/null", O_RDWR);
		dup2(devnull, STDIN_FILENO);
		dup2(devnull, STDOUT_FILENO);
		dup2(devnull, STDERR_FILENO);
		if (devnull > 2)
			close(devnull);
	}

	/* create the signal processing thread here. */
	err = pthread_create(&signal_thread_id, NULL, signal_thread, NULL);
	if (err != 0) {
		ret = 4;
		goto out;
	}

	/*
	 * Set up the signal mask which will be the default for all threads.
	 * We are handling sigint and sigterm, so block them.
	 */
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	sigaddset(&sigset, SIGTERM);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

/* Initialize espeak */
	if (initialize_espeak(&s) < 0) {
		ret = 2;
		goto out;
	}

/* open the softsynth */
	if (open_softsynth() < 0) {
		ret = 2;
		goto out;
	}

	/* Spawn our softsynth thread. */
	err = pthread_create(&softsynth_thread_id, NULL, softsynth_thread, &s);
	if (err != 0) {
		ret = 4;
		goto out;
	}

	/* Spawn our espeak-interacting thread. */
	err = pthread_create(&espeak_thread_id, NULL, espeak_thread, &s);
	if (err != 0) {
		ret = 4;
		goto out;
	}

	if (!debug && espeakup_mode == ESPEAKUP_MODE_SPEAKUP)
		(void)write(fd, &ret, 1);

	/* wait for the threads to shut down. */
	pthread_join(signal_thread_id, NULL);
	pthread_join(softsynth_thread_id, NULL);
	pthread_join(espeak_thread_id, NULL);

	if (!paused_espeak)
		espeak_Terminate();
	close_softsynth();

out:
	if (!debug && espeakup_mode == ESPEAKUP_MODE_SPEAKUP) {
		if (ret != 1)
			unlink(pidPath);
		if (ret != 0)
			(void)write(fd, &ret, 1);
		/* If ret was 0, the status byte was written before joining
		 * the threads. */
	}
	return ret;
}
