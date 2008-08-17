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

#include <espeak/speak_lib.h>

enum command_t {
	CMD_SET_FREQUENCY,
	CMD_SET_PITCH,
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

struct synth_t {
	int frequency;
	int pitch;
	int rate;
	char *voice;
	int volume;
	char *buf;
	int len;
};

extern int debug;

extern void queue_clear(void);
extern void queue_add_cmd(enum command_t cmd, enum adjust_t adj, int value);
extern void queue_add_text(char *txt, size_t length);
extern void queue_process_entry(struct synth_t *s);
extern int SynthCallback(short *wav, int numsamples, espeak_EVENT *events);
extern espeak_ERROR set_frequency(struct synth_t *s, int freq, enum adjust_t adj);
extern espeak_ERROR set_pitch (struct synth_t *s, int pitch, enum adjust_t adj);
extern espeak_ERROR set_rate (struct synth_t *s, int rate, enum adjust_t adj);
extern espeak_ERROR set_voice(struct synth_t *s);
extern espeak_ERROR set_volume (struct synth_t *s, int vol, enum adjust_t adj);
extern espeak_ERROR stop_speech(void);
extern espeak_ERROR speak_text(struct synth_t *s);
extern void open_softsynth(void);
extern void close_softsynth(void);
extern void main_loop (struct synth_t *s);

#endif
