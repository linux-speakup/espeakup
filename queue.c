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
#include <pthread.h>

#include "espeakup.h"

pthread_cond_t runner_awake = PTHREAD_COND_INITIALIZER;
pthread_mutex_t queue_guard = PTHREAD_MUTEX_INITIALIZER;

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
	pthread_mutex_lock(&queue_guard);
	assert(entry);
	entry->next = NULL;
	if (!last)
		last = entry;
	if (!first) {
		first = entry;
	} else {
		first->next = entry;
		first = first->next;
	}
	pthread_mutex_unlock(&queue_guard);
	pthread_cond_signal(&runner_awake);
}

static void free_entry(struct queue_entry_t *entry)
{
	if (entry->cmd == CMD_SPEAK_TEXT)
		free(entry->buf);
	free(entry);
}

/* Remove and return the entry at the head of the queue.
 * Return NULL if queue is empty. */

static struct queue_entry_t *queue_remove(void)
{
	struct queue_entry_t *temp = NULL;

	if(last) {
		temp = last;
		last = temp->next;

		if (!last)
			first = last;
	}

	return temp;
}

void queue_clear(void)
{
	pthread_mutex_lock(&queue_guard);
	while (last) {
		struct queue_entry_t *entry = queue_remove();
		if(entry)
			free_entry(entry);
	}
	pthread_mutex_unlock(&queue_guard);
	/* We aren't adding data to the queue, so no need to signal. */
}

void queue_add_cmd(enum command_t cmd, enum adjust_t adj, int value)
{
	struct queue_entry_t *entry;

	entry = malloc(sizeof(struct queue_entry_t));
	if (!entry) {
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
	if (!entry) {
		perror("unable to allocate memory for queue entry");
		return;
	}
	entry->cmd = CMD_SPEAK_TEXT;
	entry->adjust = ADJ_SET;
	entry->buf = strdup(txt);
	if (!entry->buf) {
		perror("unable to allocate space for text");
		free(entry);
		return;
	}
	entry->len = length;
	queue_add(entry);
}

static void queue_process_entry(struct synth_t *s)
{
	espeak_ERROR error;
	struct queue_entry_t *current = queue_remove();

	pthread_mutex_unlock(&queue_guard); /* So "reader" can go. */

	if(current) {
		switch (current->cmd) {
		case CMD_SET_FREQUENCY:
			error = set_frequency(s, current->value, current->adjust);
			break;
		case CMD_SET_PITCH:
			error = set_pitch(s, current->value, current->adjust);
			break;
		case CMD_SET_PUNCTUATION:
			error = set_punctuation(s, current->value, current->adjust);
			break;
		case CMD_SET_RATE:
			error = set_rate(s, current->value, current->adjust);
			break;
		case CMD_SET_VOICE:
			break;
		case CMD_SET_VOLUME:
			error = set_volume(s, current->value, current->adjust);
			break;
		case CMD_SPEAK_TEXT:
			s->buf = current->buf;
			s->len = current->len;
			error = speak_text(s);
			break;
		default:
			break;
		}

		free_entry(current);
	}
}

/* queue_runner is the "main" function of our secondary (queue-processing)
 * thread.
 * First, lock queue_guard, because it needs to be locked when we call
 * pthread_cond_wait on the runner_awake condition variable.
 * Next, enter an infinite loop.
 * The wait call also unlocks queue_guard, so that the other thread can
 * manipulate the queue.
 * When runner_awake is signaled, the pthread_cond_wait call re-locks
 * queue_guard, and the "queue processor" thread has access to the queue.
 * While there is an entry in the queue, call queue_process_entry.
 * queue_process_entry unlocks queue_guard after removing an item from the
 * queue, so that the main thread doesn't have to wait for us to finish
 * processing the entry.  So re-lock queue_guard after each call to
 * queue_process_entry.
 *
 * The main thread can add items to the queue in exactly two situations:
 * 1. We are waiting on runner_awake, or
 * 2. We are processing an entry that has just been removed from the queue.
*/

void *queue_runner(void *arg) {
	struct synth_t *synth = (struct synth_t *) arg;
		pthread_mutex_lock(&queue_guard);
	while(1) {
		pthread_cond_wait(&runner_awake, &queue_guard);

		while(last) {
			queue_process_entry(synth);
			pthread_mutex_lock(&queue_guard);
		}
	}

	return NULL;
}
