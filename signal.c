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

	/* install dummy handlers for the signals we want to process */
	temp.sa_handler = dummy_handler;
	sigemptyset(&temp.sa_mask);
	sigaction(SIGINT, &temp, NULL);
	sigaction(SIGTERM, &temp, NULL);

	while(should_run) {
		sigfillset(&sigset);
		sigwait(&sigset, &sig);
		switch (sig) {
		case SIGINT:
		case SIGTERM:
			printf("This is where we shut down.\n");
			should_run = 0;
			break;
		default:
			printf("espeakup caught signal %d\n", sig);
			break;
		}
	}
	return NULL;
}
