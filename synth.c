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

#include <string.h>

#include "espeakup.h"

/* multipliers and offsets */
const int frequencyMultiplier = 11;
const int pitchMultiplier = 11;
int rateMultiplier = 34;
int rateOffset = 84;
const int volumeMultiplier = 22;

espeak_ERROR set_frequency(struct synth_t *s, int freq, enum adjust_t adj)
{
	espeak_ERROR rc;

	if (adj == ADJ_DEC)
		freq = -freq;
	if (adj != ADJ_SET)
		freq += s->frequency;
	rc = espeak_SetParameter(espeakRANGE, freq * frequencyMultiplier, 0);
	if (rc == EE_OK)
		s->frequency = freq;
	return rc;
}

espeak_ERROR set_pitch(struct synth_t * s, int pitch, enum adjust_t adj)
{
	espeak_ERROR rc;

	if (adj == ADJ_DEC)
		pitch = -pitch;
	if (adj != ADJ_SET)
		pitch += s->pitch;
	rc = espeak_SetParameter(espeakPITCH, pitch * pitchMultiplier, 0);
	if (rc == EE_OK)
		s->pitch = pitch;
	return rc;
}

espeak_ERROR set_range(struct synth_t * s, int range, enum adjust_t adj)
{
	espeak_ERROR rc;

	if (adj == ADJ_DEC)
		range = -range;
	if (adj != ADJ_SET)
		range += s->range;
	rc = espeak_SetParameter(espeakRANGE, range, 0);
	if (rc == EE_OK)
		s->range = range;
	return rc;
}

espeak_ERROR set_punctuation(struct synth_t * s, int punct,
							 enum adjust_t adj)
{
	espeak_ERROR rc;

	if (adj == ADJ_DEC)
		punct = -punct;
	if (adj != ADJ_SET)
		punct += s->punct;
	rc = espeak_SetParameter(espeakPUNCTUATION, punct, 0);
	if (rc == EE_OK)
		s->punct = punct;
	return rc;
}

espeak_ERROR set_rate(struct synth_t * s, int rate, enum adjust_t adj)
{
	espeak_ERROR rc;

	if (adj == ADJ_DEC)
		rate = -rate;
	if (adj != ADJ_SET)
		rate += s->rate;
	rc = espeak_SetParameter(espeakRATE,
							 rate * rateMultiplier + rateOffset, 0);
	if (rc == EE_OK)
		s->rate = rate;
	return rc;
}

espeak_ERROR set_voice(struct synth_t * s, char *voice)
{
	espeak_ERROR rc;

	rc = espeak_SetVoiceByName(voice);
	if (rc == EE_OK)
		strcpy(s->voice, voice);
	return rc;
}

espeak_ERROR set_volume(struct synth_t * s, int vol, enum adjust_t adj)
{
	espeak_ERROR rc;

	if (adj == ADJ_DEC)
		vol = -vol;
	if (adj != ADJ_SET)
		vol += s->volume;
	rc = espeak_SetParameter(espeakVOLUME, (vol + 1) * volumeMultiplier,
							 0);
	if (rc == EE_OK)
		s->volume = vol;

	return rc;
}

espeak_ERROR stop_speech(void)
{
	return (espeak_Cancel());
}

espeak_ERROR speak_text(struct synth_t * s)
{
	espeak_ERROR rc;

	rc = espeak_Synth(s->buf, s->len + 1, 0, POS_CHARACTER, 0, 0, NULL,
					  NULL);
	return rc;
}
