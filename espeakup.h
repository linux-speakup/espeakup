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

extern int debug;

extern void process_cli(int argc, char **argv);
extern void queue_add(void *entry);
extern void queue_remove(void);
extern void *queue_peek(void);
extern void *signal_thread(void *arg);
extern void *softsynth_thread(void *arg);
extern void *espeak_thread(void *arg);
extern void select_audio_mode(void);
extern int init_audio(unsigned int rate);
extern void lock_audio_mutex(void);
extern void unlock_audio_mutex(void);
extern volatile int should_run;
extern volatile int stopped;
extern volatile int runner_must_stop;
extern int self_pipe_fds[2];
#define PIPE_READ_FD (self_pipe_fds[0])
#define PIPE_WRITE_FD (self_pipe_fds[1])

extern espeak_AUDIO_OUTPUT audio_mode;

extern pthread_cond_t runner_awake;
extern pthread_cond_t stop_acknowledged;
extern pthread_mutex_t queue_guard;
extern pthread_mutex_t acknowledge_guard;

#endif
