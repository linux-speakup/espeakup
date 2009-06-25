/*
 *  espeakup - interface which allows speakup to use espeak
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct queue_entry_t {
	void *data;
	struct queue_entry_t *next;
};

static struct queue_entry_t *head = NULL;
static struct queue_entry_t *tail = NULL;

void queue_add(void *data)
{
struct queue_entry_t *tmp;

	assert(data);
	tmp = malloc(sizeof(struct queue_entry_t));
	if (! tmp) {
		printf("Unable to allocate memory for queue entry.\n");
		return;
	}
	tmp->data = data;
	tmp->next = NULL;
	if (! tail) {
		tail = tmp;
	} else {
		tail->next = tmp;
		tail = tail->next;
	}
	if (!head)
		head = tmp;
}

void queue_remove(void)
{
	struct queue_entry_t *tmp;

	if (head) {
		tmp = head;
		head = tmp->next;
		free(tmp);
		if (!head)
			tail = head;
	}
}

void *queue_peek(void)
{
	if (head)
		return head->data;
	else
		return NULL;
}
