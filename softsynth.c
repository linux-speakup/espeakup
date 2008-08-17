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
#include <string.h>

#include "espeakup.h"

/* max buffer size */
const size_t maxBufferSize = 1025;

static int softFD = 0;

static int process_command(struct synth_t *s, char *buf, int start)
{
	char *cp;
	int value;
	enum adjust_t adj;
	enum command_t cmd;
	
	cp = buf+start;
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
		while(isdigit(*cp)) {
			value = value * 10 + (*cp - '0');
			cp++;
		}

		switch (*cp) {
		case 'f':
			cmd = CMD_SET_FREQUENCY;
			cp++;
			break;
		case 'p':
			cmd = CMD_SET_PITCH;
			cp++;
			break;
		case 's':
			cmd = CMD_SET_RATE;
			cp++;
			break;
		case 'v':
			cmd = CMD_SET_VOLUME;
			cp++;
			break;
		}
		break;
	case 24:
		cmd = CMD_FLUSH;
		stop_speech();
		queue_clear();
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

static void process_buffer (struct synth_t *s, char *buf, size_t length)
{
	int start;
	int end;
	char txtBuf[maxBufferSize];
	size_t txtLen;

	start = 0;
	end = 0;
	while (start < length) {
		while (buf[end] >= 32 && end < length)
			end++;
		if (end != start) {
			txtLen = end-start;
			strncpy (txtBuf, buf + start, txtLen);
			*(txtBuf+txtLen) = 0;
			queue_add_text(txtBuf, txtLen);
		}
		if (end < length)
			start = end = end+process_command (s, buf, end);
		else
			start = length;
	}
}

void open_softsynth(void)
{
	softFD = open ("/dev/softsynth", O_RDWR | O_NONBLOCK);
	if (softFD < 0) {
		perror("Unable to open the softsynth device");
		exit(3);
	}
}

void close_softsynth(void)
{
	close(softFD);
}

void main_loop (struct synth_t *s)
{
	fd_set set;
	int i;
	size_t length;
	char buf[maxBufferSize];
	struct timeval tv = {0, 0};
	
	while (1) {
		queue_process_entry(s);
		FD_ZERO (&set);
		FD_SET (softFD, &set);
		i = select (softFD+1, &set, NULL, NULL, &tv);
		if (i < 0) {
			if (errno == EINTR)
				continue;
			perror("Select failed");
			break;
		}

		if (! FD_ISSET(softFD, &set))
			continue;
		length = read (softFD, buf, maxBufferSize - 1);
		if (length < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			perror("Read from softsynth failed");
			break;
		}
		*(buf+length) = 0;
		process_buffer (s, buf, length);
	}
}