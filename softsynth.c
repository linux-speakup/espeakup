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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "espeakup.h"

/* max buffer size */
const size_t maxBufferSize = 1025;

/* synth flush character */
const int synthFlushChar = 0x18;

static void queue_add_cmd(enum command_t cmd, enum adjust_t adj, int value)
{
	struct espeak_entry_t *entry;

	entry = malloc(sizeof(struct espeak_entry_t));
	if (!entry) {
		perror("unable to allocate memory for queue entry");
		return;
	}
	entry->cmd = cmd;
	entry->adjust = adj;
	entry->value = value;
	pthread_mutex_lock(&queue_guard);
	queue_add((void *) entry);
	pthread_mutex_unlock(&queue_guard);
	pthread_cond_signal(&runner_awake);
}

static void queue_add_text(char *txt, size_t length)
{
	struct espeak_entry_t *entry;

	entry = malloc(sizeof(struct espeak_entry_t));
	if (!entry) {
		perror("unable to allocate memory for queue entry");
		return;
	}
	entry->cmd = CMD_SPEAK_TEXT;
	entry->adjust = ADJ_SET;
	entry->buf = strdup(txt);
	if (!entry->buf) {
		perror("unable to allocate space for text");
		free(entry);
		return;
	}
	entry->len = length;
	pthread_mutex_lock(&queue_guard);
	queue_add((void *) entry);
	pthread_mutex_unlock(&queue_guard);
	pthread_cond_signal(&runner_awake);
}

static int process_command(struct synth_t *s, char *buf, int start)
{
	char *cp;
	int value;
	enum adjust_t adj;
	enum command_t cmd;

	cp = buf + start;
	switch (*cp) {
	case 1:
		cp++;
		switch (*cp) {
		case '+':
			adj = ADJ_INC;
			cp++;
			break;
		case '-':
			adj = ADJ_DEC;
			cp++;
			break;
		default:
			adj = ADJ_SET;
			break;
		}

		value = 0;
		while (isdigit(*cp)) {
			value = value * 10 + (*cp - '0');
			cp++;
		}

		switch (*cp) {
		case 'b':
			cmd = CMD_SET_PUNCTUATION;
			break;
		case 'f':
			cmd = CMD_SET_FREQUENCY;
			break;
		case 'p':
			cmd = CMD_SET_PITCH;
			break;
		case 's':
			cmd = CMD_SET_RATE;
			break;
		case 'v':
			cmd = CMD_SET_VOLUME;
			break;
		default:
			cmd = CMD_UNKNOWN;
			break;
		}
		cp++;
		break;
	default:
		cmd = CMD_UNKNOWN;
		cp++;
		break;
	}

	if (cmd != CMD_FLUSH && cmd != CMD_UNKNOWN)
		queue_add_cmd(cmd, adj, value);

	return cp - (buf + start);
}

static void process_buffer(struct synth_t *s, char *buf, ssize_t length)
{
	int start;
	int end;
	char txtBuf[maxBufferSize];
	size_t txtLen;

	start = 0;
	end = 0;
	while (start < length) {
		while ((buf[end] < 0 || buf[end] >= ' ') && end < length)
			end++;
		if (end != start) {
			txtLen = end - start;
			strncpy(txtBuf, buf + start, txtLen);
			*(txtBuf + txtLen) = 0;
			queue_add_text(txtBuf, txtLen);
		}
		if (end < length)
			start = end = end + process_command(s, buf, end);
		else
			start = length;
	}
}

static void request_espeak_stop(void)
{
	pthread_mutex_lock(&acknowledge_guard);
	pthread_mutex_lock(&queue_guard);
	runner_must_stop = 1;
	pthread_mutex_unlock(&queue_guard);
	pthread_cond_signal(&runner_awake);	/* Wake runner, if necessary. */

	/*
	 * Runner will see runner_must_stop == 1 next time it locks 
	 * queue_guard, or when it awakens.
	 * It will lock acknowledge_guard, acknowledge the stop, and signal
	 * the reader, which will awaken.
	 */
	pthread_cond_wait(&stop_acknowledged, &acknowledge_guard);
	pthread_mutex_unlock(&acknowledge_guard);
}

void *softsynth_thread(void *arg)
{
	struct synth_t *s = (struct synth_t *) arg;
	fd_set set;
	ssize_t length;
	char buf[maxBufferSize];
	char *cp;
	int terminalFD = PIPE_READ_FD;
	int softFD;
	int greatestFD;

	/* open the softsynth. */
	softFD = open("/dev/softsynth", O_RDWR | O_NONBLOCK);
	if (softFD < 0) {
		perror("Unable to open the softsynth device");
		should_run = 0;
	}

	if (terminalFD > softFD)
		greatestFD = terminalFD;
	else
		greatestFD = softFD;
	while (should_run) {
		FD_ZERO(&set);
		FD_SET(softFD, &set);
		FD_SET(terminalFD, &set);

		if (select(greatestFD + 1, &set, NULL, NULL, NULL) < 0) {
			if (errno == EINTR)
				continue;
			perror("Select failed");
			break;
		}

		if (FD_ISSET(terminalFD, &set))
			break;

		if (!FD_ISSET(softFD, &set))
			continue;

		length = read(softFD, buf, maxBufferSize - 1);
		if (length < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			perror("Read from softsynth failed");
			break;
		}
		*(buf + length) = 0;
		cp = strrchr(buf, synthFlushChar);
		if (cp) {
			request_espeak_stop();
			printf("Returned from stop_runner\n");
			memmove(buf, cp + 1, strlen(cp + 1) + 1);
			length = strlen(buf);
		}
		process_buffer(s, buf, length);
	}
	if (softFD)
		close(softFD);
	return NULL;
}
