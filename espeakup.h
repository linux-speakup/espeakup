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

#ifndef __ESPEAKUP_H
#define __ESPEAKUP_H

/* This was added for gcc 4.3 */
#include <stddef.h>
#include <pthread.h>

#include <espeak/speak_lib.h>

#include "queue.h"

#define PACKAGE_VERSION "0.81"
#define PACKAGE_BUGREPORT "http://github.com/williamh/espeakup/issues"

enum espeakup_mode_t {
	ESPEAKUP_MODE_SPEAKUP,
	ESPEAKUP_MODE_ACSINT
};

enum command_t {
	CMD_SET_FREQUENCY,
	CMD_SET_PITCH,
	CMD_SET_PUNCTUATION,
	CMD_SET_RATE,
	CMD_SET_VOICE,
	CMD_SET_VOLUME,
	CMD_SPEAK_TEXT,
	CMD_FLUSH,
	CMD_UNKNOWN,
};

enum adjust_t {
	ADJ_DEC,
	ADJ_SET,
	ADJ_INC,
};

struct espeak_entry_t {
	enum command_t cmd;
	enum adjust_t adjust;
	int value;
	char *buf;
	int len;
};

struct synth_t {
	int frequency;
	int pitch;
	int punct;
	int rate;
	char voice[10];
	int volume;
	char *buf;
	int len;
};

extern struct queue_t *synth_queue;
extern int debug;
extern enum espeakup_mode_t espeakup_mode;

extern void process_cli(int argc, char **argv);
extern void *signal_thread(void *arg);
extern int initialize_espeak(struct synth_t *s);
extern void *espeak_thread(void *arg);
extern int open_softsynth(void);
extern void close_softsynth(void);
extern void *softsynth_thread(void *arg);
extern volatile int should_run;
extern volatile int stop_requested;
extern int self_pipe_fds[2];
#define PIPE_READ_FD (self_pipe_fds[0])
#define PIPE_WRITE_FD (self_pipe_fds[1])

extern pthread_cond_t runner_awake;
extern pthread_cond_t stop_acknowledged;
extern pthread_mutex_t queue_guard;

#endif
