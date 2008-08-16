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
const int maxBufferSize = 1025;

static int softFD = 0;

static int process_command(struct synth_t *s, char *buf, int start)
{
	int rc;
	char value;
	char param;
	
	rc = 1;
	switch (buf[start]) {
	case 1:
		if (buf[start+1] == '+' || buf[start+1] == '-') {
			value = buf[start+2]-'0';
			param = buf[start+3];
			if (buf[start+1] == '-')
				value = -value;
			switch (param) {
			case 'f':
				s->frequency += value;
				set_frequency (s);
				break;
			case 'p':
				s->pitch += value;
				set_pitch (s);
				break;
			case 's':
				s->rate += value;
				set_rate (s);
				break;
			case 'v':
				s->volume += value;
				set_volume (s);
				break;
			}
			rc = 4;
		} else if (buf[start+1] >= '0' && buf[start+1] <= '9') {
			value = buf[start+1]-'0';
			param = buf[start+2];
			switch (param) {
			case 'f':
				s->frequency = value;
				set_frequency (s);
				break;
			case 'p':
				s->pitch = value;
				set_pitch (s);
				break;
			case 's':
				s->rate = value;
				set_rate (s);
				break;
			case 'v':
				s->volume = value;
				set_volume (s);
				break;
			}
		}
		rc = 3;
		break;
	case 24:
		stop_speech();
		break;
	}
	return rc;
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
			s->buf = txtBuf;
			s->len = txtLen;
			speak_text(s);
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
	
	while (1) {
		FD_ZERO (&set);
		FD_SET (softFD, &set);
		i = select (softFD+1, &set, NULL, NULL, NULL);
		if (i < 0) {
			if (errno == EINTR)
				continue;
			perror("Select failed");
			break;
		}

		length = read (softFD, buf, maxBufferSize - 1);
		if (length < 0) {
			perror("Read from softsynth failed");
			break;
		}
		*(buf+length) = 0;
		process_buffer (s, buf, length);
	}
}
