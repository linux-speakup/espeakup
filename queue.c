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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "espeakup.h"

struct queue_entry_t {
	enum command_t cmd;
	enum adjust_t adjust;
	int value;
	char *buf;
	int len;
	struct queue_entry_t *next;
};

static struct queue_entry_t *first = NULL;
static struct queue_entry_t *last = NULL;

static void queue_add(struct queue_entry_t *entry)
{
	assert(entry);
	entry->next = NULL;
	if (! last)
		last = entry;
	if (! first) {
		first = entry;
	} else {
		first->next = entry;
		first = first->next;
	}
}

static void queue_remove(void)
{
	struct queue_entry_t *temp;

	assert(last);
	temp = last;
	last = temp->next;
	if (temp->cmd == CMD_SPEAK_TEXT)
		free(temp->buf);
	free(temp);
	if (! last)
		first = last;
}

void queue_clear(void)
{
	while(last)
		queue_remove();
}

void queue_add_cmd(enum command_t cmd, enum adjust_t adj, int value)
{
	struct queue_entry_t *entry;

	entry = malloc(sizeof(struct queue_entry_t));
	if (! entry) {
		perror("unable to allocate memory for queue entry");
		return;
	}
	entry->cmd = cmd;
	entry->adjust = adj;
	entry->value = value;
	queue_add(entry);
}

void queue_add_text(char *txt, size_t length)
{
	struct queue_entry_t *entry;

	entry = malloc(sizeof(struct queue_entry_t));
	if (! entry) {
		perror("unable to allocate memory for queue entry");
		return;
	}
	entry->cmd = CMD_SPEAK_TEXT;
	entry->adjust = ADJ_SET;
	entry->buf = strdup(txt);
	if (! entry->buf) {
		perror("unable to allocate space for text");
		free(entry);
		return;
	}
	entry->len = length;
	queue_add(entry);
}

void queue_process_entry(struct synth_t *s)
{
	espeak_ERROR error;

	if(! last)
		return;

	switch (last->cmd) {
	case CMD_SET_FREQUENCY:
		error = set_frequency(s, last->value, last->adjust);
		break;
	case CMD_SET_PITCH:
		error = set_pitch(s, last->value, last->adjust);
		break;
	case CMD_SET_RATE:
		error = set_rate(s, last->value, last->adjust);
		break;
	case CMD_SET_VOICE:
		break;
	case CMD_SET_VOLUME:
		error = set_volume(s, last->value, last->adjust);
		break;
	case CMD_SPEAK_TEXT:
		s->buf = last->buf;
		s->len = last->len;
		error = speak_text(s);
		break;
	}
	if (error == EE_OK)
		queue_remove();
}
