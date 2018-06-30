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
#include <sys/select.h>
#include <string.h>
#include <ctype.h>

#include "espeakup.h"
#include "stringhandling.h"

/* max buffer size */
static const size_t maxBufferSize = 16 * 1024 + 1;

/* synth flush character */
static const int synthFlushChar = 0x18;

static int softFD = 0;

/* Text accumulator: */
char *textAccumulator;
int textAccumulator_l;

static void queue_add_cmd(enum command_t cmd, enum adjust_t adj, int value)
{
	struct espeak_entry_t *entry;
	int added = 0;

	entry = allocMem(sizeof(struct espeak_entry_t));
	entry->cmd = cmd;
	entry->adjust = adj;
	entry->value = value;
	pthread_mutex_lock(&queue_guard);
	added = queue_add(synth_queue, (void *) entry);
	if (!added)
		free(entry);
	else
		pthread_cond_signal(&runner_awake);
	pthread_mutex_unlock(&queue_guard);
}

static void queue_add_text(char *txt, size_t length)
{
	struct espeak_entry_t *entry;
	int added = 0;

	entry = allocMem(sizeof(struct espeak_entry_t));
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
	added = queue_add(synth_queue, (void *) entry);
	if (!added) {
		free(entry->buf);
		free(entry);
	} else {
		pthread_cond_signal(&runner_awake);
	}

	pthread_mutex_unlock(&queue_guard);
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
		case 'P':
			cmd = CMD_PAUSE;
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

	if (cmd != CMD_FLUSH && cmd != CMD_UNKNOWN) {
		if (espeakup_mode == ESPEAKUP_MODE_ACSINT
			&& textAccumulator_l != 0) {
			queue_add_text(textAccumulator, textAccumulator_l);
			free(textAccumulator);
			textAccumulator = initString(&textAccumulator_l);
		}
		queue_add_cmd(cmd, adj, value);
	}

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

static void process_buffer_acsint(struct synth_t *s, char *buf,
								  ssize_t length)
{
	int start = 0;
	int i;
	int flushIt = 0;

	while (start < length) {
		for (i = start; i < length; i++) {
			if (buf[i] == '\r' || buf[i] == '\n')
				flushIt = 1;
			if (buf[i] >= 0 && buf[i] < ' ')
				break;
		}
		if (i > start)
			stringAndBytes(&textAccumulator, &textAccumulator_l,
						   buf + start, i - start);
		if (flushIt) {
			if (textAccumulator != EMPTYSTRING) {
				queue_add_text(textAccumulator, textAccumulator_l);
				free(textAccumulator);
				textAccumulator = initString(&textAccumulator_l);
			}
			flushIt = 0;
		}
		if (i < length)
			start = i = i + process_command(s, buf, i);
		else
			start = length;
	}
}

static void request_espeak_stop(void)
{
	pthread_mutex_lock(&queue_guard);
	stop_requested = 1;
	pthread_cond_signal(&runner_awake);	/* Wake runner, if necessary. */
	while (should_run && stop_requested)
		pthread_cond_wait(&stop_acknowledged, &queue_guard);	/* wait for acknowledgement. */
	pthread_mutex_unlock(&queue_guard);
}

int open_softsynth(void)
{
	int rc = 0;
	/* If we're in acsint mode, we read from stdin.  No need to open. */
	if (espeakup_mode == ESPEAKUP_MODE_ACSINT) {
		softFD = STDIN_FILENO;
		return 0;
	}

	/* open the softsynth. */
	softFD = open("/dev/softsynthu", O_RDWR | O_NONBLOCK);
	if (softFD < 0 && errno == ENOENT)
		/* Kernel without unicode support?  Try without unicode.  */
		softFD = open("/dev/softsynth", O_RDWR | O_NONBLOCK);
	if (softFD < 0) {
		perror("Unable to open the softsynth device");
		rc = -1;
	}
	return rc;
}

void close_softsynth(void)
{
	if (softFD)
		close(softFD);
}

void *softsynth_thread(void *arg)
{
	struct synth_t *s = (struct synth_t *) arg;
	fd_set set;
	ssize_t length;
	char buf[maxBufferSize];
	char *cp;
	int terminalFD = PIPE_READ_FD;
	int greatestFD;

	textAccumulator = initString(&textAccumulator_l);

	if (terminalFD > softFD)
		greatestFD = terminalFD;
	else
		greatestFD = softFD;
	pthread_mutex_lock(&queue_guard);
	while (should_run) {
		pthread_mutex_unlock(&queue_guard);
		FD_ZERO(&set);
		FD_SET(softFD, &set);
		FD_SET(terminalFD, &set);

		if (select(greatestFD + 1, &set, NULL, NULL, NULL) < 0) {
			if (errno == EINTR) {
				pthread_mutex_lock(&queue_guard);
				continue;
			}
			perror("Select failed");
			pthread_mutex_lock(&queue_guard);
			break;
		}

		if (FD_ISSET(terminalFD, &set)) {
			pthread_mutex_lock(&queue_guard);
			break;
		}

		if (!FD_ISSET(softFD, &set)) {
			pthread_mutex_lock(&queue_guard);
			continue;
		}

		length = read(softFD, buf, maxBufferSize - 1);
		if (length < 0) {
			if (errno == EAGAIN || errno == EINTR) {
				pthread_mutex_lock(&queue_guard);
				continue;
			}
			perror("Read from softsynth failed");
			pthread_mutex_lock(&queue_guard);
			break;
		}
		*(buf + length) = 0;
		cp = strrchr(buf, synthFlushChar);
		if (cp) {
			request_espeak_stop();
			memmove(buf, cp + 1, strlen(cp + 1) + 1);
			length = strlen(buf);
		}
		if (espeakup_mode == ESPEAKUP_MODE_SPEAKUP)
			process_buffer(s, buf, length);
		else
			process_buffer_acsint(s, buf, length);
		pthread_mutex_lock(&queue_guard);
	}
	pthread_cond_signal(&runner_awake);
	pthread_mutex_unlock(&queue_guard);
	return NULL;
}
