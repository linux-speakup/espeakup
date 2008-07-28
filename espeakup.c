/*
 *  espeakup - connector so that speakup can use espeak.
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

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <espeak/speak_lib.h>

/* multipliers and offsets */
const int pitchMultiplier = 10;
const int rateMultiplier = 32;
const int rateOffset = 80;
const int volumeMultiplier = 10;

struct synth_t {
	int fd;
	int pitch;
	int rate;
	int volume;
	char buf[1025];
};

int SynthCallback(short *wav, int numsamples, espeak_EVENT *events)
{
	return 0;
}

static void set_pitch (struct synth_t *s)
{
	espeak_SetParameter(espeakPITCH, s->pitch * pitchMultiplier, 0);
}

static void set_rate (struct synth_t *s)
{
espeak_SetParameter(espeakRATE, s->rate * rateMultiplier + rateOffset, 0);
}

static void set_volume (struct synth_t *s)
{
	espeak_SetParameter(espeakVOLUME, s->volume * volumeMultiplier, 0);
}

static void stop_speech(void)
{
	espeak_Cancel();
}

static void speak_text(struct synth_t *s)
{
	if (! strlen(s->buf))
		return;
	espeak_Synth(s->buf, strlen(s->buf), 0, POS_CHARACTER, 0, 0, NULL, s);
	strcpy(s->buf, "");
}

static int process_command(struct synth_t *s, char *buf, int start)
{
	char value;
	char param;
	
	switch (buf[start]) {
	case 1:
		if (buf[start+1] == '+' || buf[start+1] == '-') {
			value = buf[start+2]-'0';
			param = buf[start+3];
			if (buf[start+1] == '-')
				value = -value;
			switch (param) {
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
			return 4;
		} else if (buf[start+1] >= '0' && buf[start+1] <= '9') {
			value = buf[start+1]-'0';
			param = buf[start+2];
			switch (param) {
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
		return 3;
	case 24:
		stop_speech();
		return 1;
	}
	return 1;
}

static void process_data (struct synth_t *s)
{
	int length;
	int start;
	int end;
	char buf[1025];
	char tmp_buf[1025];

	length = read (s->fd, buf, 1024);
	start = 0;
	end = 0;
	while (start < length) {
		while (buf[end] >= 32 && end < length)
			end++;
		if (end != start) {
			strncpy (tmp_buf, buf + start, end-start);
			tmp_buf[end-start] = 0;
			strcat(s->buf, tmp_buf);
		}
		if (end < length)
			start = end = end+process_command (s, buf, end);
		else
			start = length;
	}
	speak_text (s);
}

static void main_loop (struct synth_t *s)
{
	fd_set set;
	struct timeval tv;
	struct timeval timeout = {0, 20000};
	int i;
	
	while (1) {
		FD_ZERO (&set);
		FD_SET (s->fd, &set);
		tv = timeout;
		i = select (s->fd+1, &set, NULL, NULL, &tv);
		if (i)
			process_data (s);
	}
}

int main ()
{
	struct synth_t s;

	/* become a daemon */
	daemon(0, 1);

	/* open the softsynth. */
	s.fd = open ("/dev/softsynth", O_RDWR | O_NONBLOCK);
	if (s.fd < 0) {
		printf("Unable to open the softsynth device.\n");
		return -1;
	}

	/* initialize espeak */
	espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL, 0);
	espeak_SetSynthCallback(SynthCallback);

	/* Setup initial voice parameters */
	s.pitch = 5;
	s.rate = 5;
	s.volume = 5;
	strcpy(s.buf,"");
	set_pitch (&s);
	set_rate (&s);
	set_volume(&s);

	/* run the main loop */
	main_loop (&s);

	/* shutdown espeak and close the softsynth */
	espeak_Terminate();
	close(s.fd);

	return 0;
}
