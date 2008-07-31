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

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <espeak/speak_lib.h>

/* command line options */
const char *cliOptions = "d";

/* max buffer size */
const int maxBufferSize = 1025;

/* multipliers and offsets */
const int pitchMultiplier = 10;
const int rateMultiplier = 32;
const int rateOffset = 80;
const int volumeMultiplier = 10;

struct synth_t {
	int pitch;
	int rate;
	char voice[40];
	int volume;
	int len;
	char *buf;
};

int debug = 0;
int softFD;

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

static void set_voice(struct synth_t *s)
{
	espeak_SetVoiceByName(s->voice);
}

static void set_volume (struct synth_t *s)
{
	espeak_SetParameter(espeakVOLUME, (s->volume+1) * volumeMultiplier, 0);
}

static void stop_speech(void)
{
	espeak_Cancel();
}

static void speak_text(struct synth_t *s)
{
	espeak_Synth(s->buf, s->len + 1, 0, POS_CHARACTER, 0, 0, NULL, NULL);
	for (s->len = 0; s->len < maxBufferSize; s->len++)
		*(s->buf+s->len) = 0;
	s->len = 0;
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

static void process_data (struct synth_t *s, char *buf, size_t length)
{
	int start;
	int end;
	char txtBuf[maxBufferSize];
	size_t txtLen;

	for(start = 0; start < maxBufferSize; start++)
		*(txtBuf+start) = 0;
	
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
			speak_text (s);
		}
		if (end < length)
			start = end = end+process_command (s, buf, end);
		else
			start = length;
	}
}

static void main_loop (struct synth_t *s)
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
			printf("Select failed!\n");
			break;
		}

		length = read (softFD, buf, maxBufferSize - 1);
		if (length < 0) {
			printf("Read from softsynth failed!\n");
			break;
		}
		*(buf+length) = 0;
		process_data (s, buf, length);
	}
}

int main (int argc, char **argv)
{
	struct synth_t s = {
		.pitch = 5,
		.rate = 5,
		.voice = "default",
		.volume = 5,
	};

	int opt;

	while ((opt = getopt(argc, argv, cliOptions)) != -1) {
		switch(opt) {
		case 'd':
			debug = 1;
			break;
		default:
			printf("usage: %s [-d]\n", argv[0]);
			exit(1);
			break;
		}
	}

	/* become a daemon */
	if (! debug)
		daemon(0, 1);

	/* open the softsynth. */
	softFD = open ("/dev/softsynth", O_RDWR | O_NONBLOCK);
	if (softFD < 0) {
		printf("Unable to open the softsynth device.\n");
		return -1;
	}

	/* initialize espeak */
	espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL, 0);
	espeak_SetSynthCallback(SynthCallback);

	/* Setup initial voice parameters */
	set_voice(&s);
	set_pitch (&s);
	set_rate (&s);
	set_volume(&s);

	/* run the main loop */
	main_loop (&s);

	/* shutdown espeak and close the softsynth */
	espeak_Terminate();
	close(softFD);

	return 0;
}
