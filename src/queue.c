/*
 *  espeakup - interface which allows speakup to use espeak-ng
 *
 * Note that these functions are meant to be used in either a single or
 * multi-threaded environment, so they know nothing about mutexes, etc.
 * Handling this is up to the caller.
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
#include <stdlib.h>

#include "stringhandling.h"

struct queue_entry_t {
	void *data;
	struct queue_entry_t *next;
};

struct queue_t {
	struct queue_entry_t *head;
	struct queue_entry_t *tail;
};

struct queue_t *new_queue(void)
{
	struct queue_t *q = allocMem(sizeof(struct queue_t));
	q->head = NULL;
	q->tail = NULL;
	return q;
}

int queue_add(struct queue_t *q, void *data)
{
	struct queue_entry_t *tmp;

	assert(data);
	tmp = allocMem(sizeof(struct queue_entry_t));
	tmp->data = data;
	tmp->next = NULL;
	if (!q->tail) {
		q->tail = tmp;
	} else {
		q->tail->next = tmp;
		q->tail = q->tail->next;
	}
	if (!q->head)
		q->head = tmp;
	return 1;
}

void *queue_remove(struct queue_t *q)
{
	void *data = NULL;
	struct queue_entry_t *tmp;

	if (q->head) {
		tmp = q->head;
		data = tmp->data;
		q->head = tmp->next;
		free(tmp);
		if (!q->head)
			q->tail = q->head;
	}
	return data;
}

void *queue_peek(struct queue_t *q)
{
	if (q->head)
		return q->head->data;
	else
		return NULL;
}
