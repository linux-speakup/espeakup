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

#ifndef __STRINGHANDLING_H
#define __STRINGHANDLING_H

#include <stddef.h>

extern char *EMPTYSTRING;

void *allocMem(size_t n);
void *reallocMem(void *p, size_t n);
char *dupeString(char *s);
char *initString(int *l);
void stringAndString(char **s, int *l, const char *t);
void stringAndBytes(char **s, int *l, const char *t, int cnt);

#endif
