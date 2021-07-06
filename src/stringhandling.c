/*
 *  espeakup - interface which allows speakup to use espeak-ng
 *
 *  Copyright (C) 2011 William Hubbs
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

/*
 * These string routines were borrowed from edbrowse, which was
 * originally written by Karl Dahlke, and is being currently maintained
 * by Christopher Brannon.
 * I would like to thank both of them for their work on this software.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *EMPTYSTRING = "";

void *allocMem(size_t n)
{
	void *s;
	if (!n)
		return EMPTYSTRING;
	if (!(s = malloc(n))) {
		fprintf(stderr, "Out of memory!\n");
		exit(1);
	}
	return s;
}

void *reallocMem(void *p, size_t n)
{
	void *s;
	if (!n) {
		fprintf(stderr, "Trying to reallocate memory with size of 0.\n");
		exit(1);
	}
	if (!p) {
		fprintf(stderr, "realloc called with a NULL pointer!\n");
		exit(1);
	}
	if (p == EMPTYSTRING)
		return allocMem(n);
	if (!(s = realloc(p, n))) {
		fprintf(stderr, "Failed to allocate memory.\n");
		exit(1);
	}
	return s;
}

char *dupeString(char *s)
{
	char *c;

	if (!(c = strdup(s))) {
		fprintf(stderr, "Out of memory!\n");
		exit(1);
	}
	return c;
}

char *initString(int *l)
{
	*l = 0;
	return EMPTYSTRING;
}

void stringAndString(char **s, int *l, const char *t)
{
	char *p = *s;
	int oldlen, newlen, x;
	oldlen = *l;
	newlen = oldlen + strlen(t);
	*l = newlen;
	++newlen;     // room for the 0
	x = oldlen ^ newlen;
	if (x > oldlen) {     // must realloc
		newlen |= (newlen >> 1);
		newlen |= (newlen >> 2);
		newlen |= (newlen >> 4);
		newlen |= (newlen >> 8);
		newlen |= (newlen >> 16);
		p = reallocMem(p, newlen);
		*s = p;
	}
	strcpy(p + oldlen, t);
}

void stringAndBytes(char **s, int *l, const char *t, int cnt)
{
	char *p = *s;
	int oldlen, newlen, x;
	oldlen = *l;
	newlen = oldlen + cnt;
	*l = newlen;
	++newlen;
	x = oldlen ^ newlen;
	if (x > oldlen) {     // must realloc
		newlen |= (newlen >> 1);
		newlen |= (newlen >> 2);
		newlen |= (newlen >> 4);
		newlen |= (newlen >> 8);
		newlen |= (newlen >> 16);
		p = reallocMem(p, newlen);
		*s = p;
	}
	memcpy(p + oldlen, t, cnt);
	p[oldlen + cnt] = 0;
}
