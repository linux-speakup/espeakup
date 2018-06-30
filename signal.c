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

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#define STOP_MSG "s"

#include "espeakup.h"

/*
 * We install a dummy signal handler to let the o/s know that we
 * do not want the default action to be performed since we are
 * handling the signal.
 */
static void dummy_handler(int sig)
{
}

void *signal_thread(void *arg)
{
	struct sigaction temp;
	sigset_t sigset;
	int sig;

	memset(&temp, 0, sizeof (struct sigaction));
	/* install dummy handlers for the signals we want to process */
	temp.sa_handler = dummy_handler;
	sigemptyset(&temp.sa_mask);
	sigaction(SIGINT, &temp, NULL);
	sigaction(SIGTERM, &temp, NULL);

	pthread_mutex_lock(&queue_guard);
	while (should_run) {
		pthread_mutex_unlock(&queue_guard);
		sigfillset(&sigset);
		sigwait(&sigset, &sig);
		switch (sig) {
		case SIGINT:
		case SIGTERM:
			pthread_mutex_lock(&queue_guard);
			should_run = 0;
			pthread_mutex_unlock(&queue_guard);
			break;
		default:
			printf("espeakup caught signal %d\n", sig);
			break;
		}
		pthread_mutex_lock(&queue_guard);
	}
	pthread_mutex_unlock(&queue_guard);
	/* Tell the reader to stop, if it is in a select() call. */
	write(PIPE_WRITE_FD, STOP_MSG, strlen(STOP_MSG));
	return NULL;
}
