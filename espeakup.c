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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <espeak/speak_lib.h>

/* program version */
const char *Version = "0.1";

/* command line options */
const char *shortOptions = "dhv";

/* max buffer size */
const int maxBufferSize = 1025;

/* path to our pid file */
const char *pidPath = "/var/run/espeakup.pid";

/* multipliers and offsets */
const int frequencyMultiplier = 11;
const int pitchMultiplier = 11;
const int rateMultiplier = 32;
const int rateOffset = 80;
const int volumeMultiplier = 11;

struct synth_t {
	int frequency;
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

static void set_frequency (struct synth_t *s)
{
	espeak_SetParameter(espeakRANGE, s->frequency * frequencyMultiplier, 0);
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
			return 4;
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
		if (! kill(pid, 0) || errno != ESRCH)
			rc = 1;
	}
	return rc;
}

int create_pid_file(void)
{
	FILE *pidFile;

	pidFile = fopen(pidPath, "w");
	if (! pidFile)
		return -1;

	fprintf(pidFile, "%d\n", getpid());
	fclose(pidFile);
	return 0;
}
 
void espeakup_sighandler(int sig)
{
	if (debug)
		printf("Caught signal %i\n", sig);
	
	/* shutdown espeak and close the softsynth */
	espeak_Terminate();
	close(softFD);

	if (! debug)
		unlink(pidPath);
	exit(0);
}

void show_help()
{
	printf("Usage:  espeakup [-d] [-h] [-v]\n\n");
	printf("-d\tDebug mode (stay in the foreground)\n");
	printf("-h\tShow this help\n");
	printf("-v\tDisplay the software version.\n");
}

void show_version(void)
{
	printf("espeakup %s\n", Version);
	printf("Copyright (C) 2008 William Hubbs\n");
	printf("License GPLv3+: GNU GPL version 3 or later\n");
	printf("You are free to change and redistribute this software.\n");
}

int main(int argc, char **argv)
{
	struct synth_t s = {
		.frequency = 5,
		.pitch = 5,
		.rate = 5,
		.voice = "default",
		.volume = 5,
	};

	int opt;

	while ((opt = getopt(argc, argv, shortOptions)) != -1) {
		switch(opt) {
		case 'd':
			debug = 1;
			break;
		case 'h':
			show_help();
			return 0;
			break;
		case 'v':
			show_version();
			return 0;
			break;
		default:
			show_help();
			return 0;
			break;
		}
	}

	/* become a daemon */
	if (! debug)
		daemon(0, 1);

	/* make sure espeakup is not already running */
	if (! debug && espeakup_is_running()) {
		printf("Espeakup is already running!\n");
		return 1;
	}

	/* write our pid file. */
	if (! debug && create_pid_file() < 0) {
		printf("Unable to create pid file!\n");
		return 2;
	}

	/* open the softsynth. */
	softFD = open ("/dev/softsynth", O_RDWR | O_NONBLOCK);
	if (softFD < 0) {
		printf("Unable to open the softsynth device.\n");
		return 3;
	}

	/* initialize espeak */
	espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL, 0);
	espeak_SetSynthCallback(SynthCallback);

	/* Setup initial voice parameters */
	set_voice(&s);
	set_frequency (&s);
	set_pitch (&s);
	set_rate (&s);
	set_volume(&s);

	/* register signal handler */
	signal(SIGINT, espeakup_sighandler);
	signal(SIGTERM, espeakup_sighandler);

	/* run the main loop */
	main_loop (&s);

	return 0;
}
