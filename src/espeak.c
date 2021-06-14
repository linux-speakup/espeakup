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

#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alsa/asoundlib.h>
#include <math.h>

#include "espeakup.h"

/* default voice settings */
const int defaultFrequency = 5;
const int defaultPitch = 5;
const int defaultRange = 5;
const int defaultRate = 2;
const int defaultVolume = 5;
char *defaultVoice = NULL;
int alsaVolume = 0;

/* multipliers and offsets */
const int frequencyMultiplier = 11;
const int pitchMultiplier = 11;
const int rangeMultiplier = 11;
const int rateMultiplier = 41;
const int rateOffset = 80;
const int volumeMultiplier = 22;

volatile int stop_requested = 0;
int paused_espeak = 1;

static int callback(short *wav, int numsamples, espeak_EVENT * events)
{
	int i;
	for (i = 0; events[i].type !=  espeakEVENT_LIST_TERMINATED; i++) {
		if (events[i].type == espeakEVENT_MARK) {
			int mark = atoi(events[i].id.name);
			if ((mark < 0) || (mark > 255))
				continue;
			softsynth_reportindex(mark);
		}
	}
	return 0;
}

static espeak_ERROR set_frequency(struct synth_t *s, int freq,
								  enum adjust_t adj)
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

static espeak_ERROR set_pitch(struct synth_t *s, int pitch,
							  enum adjust_t adj)
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

static espeak_ERROR set_range(struct synth_t *s, int range,
							  enum adjust_t adj)
{
	espeak_ERROR rc;

	if (adj == ADJ_DEC)
		range = -range;
	if (adj != ADJ_SET)
		range += s->range;
	rc = espeak_SetParameter(espeakRANGE, range * rangeMultiplier, 0);
	if (rc == EE_OK)
		s->range = range;
	return rc;
}

static espeak_ERROR set_punctuation(struct synth_t *s, int punct,
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

static espeak_ERROR set_rate(struct synth_t *s, int rate,
							 enum adjust_t adj)
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

static espeak_ERROR set_voice(struct synth_t *s, char *voice)
{
	espeak_ERROR rc;
	espeak_VOICE voice_select;

	rc = espeak_SetVoiceByName(voice);
	if (rc != EE_OK)
	{
		memset(&voice_select, 0, sizeof(voice_select));
		voice_select.languages = voice;
		rc = espeak_SetVoiceByProperties(&voice_select);
	}
	if (rc == EE_OK)
		strcpy(s->voice, voice);
	return rc;
}

static void set_alsa_volume(int vol)
{
	snd_mixer_t *m;
	snd_mixer_elem_t *e;
	int err;

	err = snd_mixer_open(&m, 0);
	if (err < 0)
	{
		fprintf(stderr, "ALSA mixer open error: %s\n", snd_strerror(err));
		return;
	}

	err = snd_mixer_attach(m, "default");
	if (err < 0)
	{
		fprintf(stderr, "ALSA mixer attach error: %s\n", snd_strerror(err));
		return;
	}
	err = snd_mixer_selem_register(m, NULL, NULL);
	if (err < 0)
	{
		fprintf(stderr, "ALSA mixer load error: %s\n", snd_strerror(err));
		return;
	}
	err = snd_mixer_load(m);
	if (err < 0)
	{
		fprintf(stderr, "ALSA mixer load error: %s\n", snd_strerror(err));
		return;
	}

	/* Turn vol value to volume %.
	 * We do not want to soften that much with ALSA, espeak is already
	 * doing it. We want the default value (5) to be the usual default
	 * volume (80%), and make higher values increase ALSA volume, up to
	 * 100%. */

	int volume = (vol+1) * 50 / 10 + 50;

	for (e = snd_mixer_first_elem(m); e; e = snd_mixer_elem_next(e))
	{
		if (snd_mixer_elem_get_type(e) != SND_MIXER_ELEM_SIMPLE)
			continue;
		if (snd_mixer_selem_is_enumerated(e))
			continue;

		if (snd_mixer_selem_has_playback_switch(e)) {
			snd_mixer_selem_set_playback_switch_all(e, 1);
		}

		if (snd_mixer_selem_has_playback_volume(e)) {
			long min, max, set;

			err = snd_mixer_selem_get_playback_dB_range(e, &min, &max);
			if (err == 0 && min < max) {
				if (max - min < 2400) {
					/* 24dB amplitude is too small for using a logscale */
					set = min + volume * (max-min) / 100;
				} else {
					/* Use a logscale */
					double volf = volume / 100.;
					if (min != SND_CTL_TLV_DB_GAIN_MUTE)
					{
						double minf = pow(10, (min-max) / 6000.);
						volf = volf * (1 - minf) + minf;
					}
					set = 6000. * log10(volf) + max;
				}
				snd_mixer_selem_set_playback_dB_all(e, set, 0);
			} else {
				/* No dB setting, try a linear scale */
				snd_mixer_selem_get_playback_volume_range(e, &min, &max);
				set = min + volume * (max-min) / 100;
				snd_mixer_selem_set_playback_volume_all(e, set);
			}
		}
	}
}

static espeak_ERROR set_volume(struct synth_t *s, int vol,
							   enum adjust_t adj)
{
	espeak_ERROR rc;

	if (adj == ADJ_DEC)
		vol = -vol;
	if (adj != ADJ_SET)
		vol += s->volume;
	rc = espeak_SetParameter(espeakVOLUME, (vol + 1) * volumeMultiplier,
							 0);
	if (rc == EE_OK)
	{
		s->volume = vol;
		if (alsaVolume)
			set_alsa_volume(vol);
	}

	return rc;
}

static espeak_ERROR stop_speech(void)
{
	espeak_ERROR rc;

	rc = espeak_Cancel();
	return rc;
}

static espeak_ERROR speak_text(struct synth_t *s)
{
	espeak_ERROR rc;
	int synth_mode = 0;

	if (espeakup_mode == ESPEAKUP_MODE_ACSINT)
		synth_mode |= espeakSSML;

	if (espeakup_mode == ESPEAKUP_MODE_SPEAKUP && (s->len == 1)) {
		char *buf;
		int n;
		if (s->buf[0] == ' ')
			n = asprintf(&buf,
				     "<say-as interpret-as=\"tts:char\">&#32;</say-as>");
		else
			n = asprintf(&buf,
				     "<say-as interpret-as=\"characters\">%c</say-as>",
				     s->buf[0]);
		if (n == -1) {
			/* D'oh.  Not much to do on allocation failure.
			 * Perhaps espeak will happen to say the character */
			rc = espeak_Synth(s->buf, s->len + 1, 0, POS_CHARACTER,
					  0, synth_mode, NULL, NULL);
		} else {
			rc = espeak_Synth(buf, n + 1, 0, POS_CHARACTER, 0,
					  espeakSSML, NULL, NULL);
			free(buf);
		}
	} else
		rc = espeak_Synth(s->buf, s->len + 1, 0, POS_CHARACTER, 0,
				  synth_mode, NULL, NULL);
	return rc;
}

static void free_espeak_entry(struct espeak_entry_t *entry)
{
	assert(entry);
	if (entry->cmd == CMD_SPEAK_TEXT)
		free(entry->buf);
	free(entry);
}

static void synth_queue_clear()
{
	struct espeak_entry_t *current;

	while (queue_peek(synth_queue)) {
		current = (struct espeak_entry_t *) queue_remove(synth_queue);
		free_espeak_entry(current);
	}
}

static void reinitialize_espeak(struct synth_t *s)
{
	int rate;

	/* Re-initialize espeak */
	rate = espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL, 0);
	if (rate < 0) {
		fprintf(stderr, "Unable to initialize espeak.\n");
		return;
	}

	espeak_SetSynthCallback(callback);

	/* Set parameters again */
	espeak_SetVoiceByName(s->voice);
	espeak_SetParameter(espeakRANGE, s->frequency * frequencyMultiplier, 0);
	espeak_SetParameter(espeakPITCH, s->pitch * pitchMultiplier, 0);
	espeak_SetParameter(espeakRATE, s->rate * rateMultiplier + rateOffset, 0);
	espeak_SetParameter(espeakVOLUME, (s->volume + 1) * volumeMultiplier, 0);
	espeak_SetParameter(espeakCAPITALS, 0, 0);
	paused_espeak = 0;
	return;
}

static void queue_process_entry(struct synth_t *s)
{
	espeak_ERROR error;
	char markbuff[50];
	static struct espeak_entry_t *current = NULL;

	if (current != queue_peek(synth_queue)) {
		if (current)
			free_espeak_entry(current);
		current = (struct espeak_entry_t *) queue_remove(synth_queue);
	}
	pthread_mutex_unlock(&queue_guard);

	if (current->cmd != CMD_PAUSE && paused_espeak) {
		reinitialize_espeak(s);
	}

	switch (current->cmd) {
	case CMD_SET_FREQUENCY:
		error = set_frequency(s, current->value, current->adjust);
		break;
	case CMD_SET_MARK:
		snprintf(markbuff, sizeof(markbuff), "<mark name=\"%d\"/>", current->value);
		error = espeak_Synth(markbuff, strlen(markbuff)+1, 0, POS_CHARACTER,
				     0, espeakSSML, NULL, NULL);
		break;
	case CMD_SET_PITCH:
		error = set_pitch(s, current->value, current->adjust);
		break;
	case CMD_SET_RANGE:
		error = set_range(s, current->value, current->adjust);
		break;
	case CMD_SET_PUNCTUATION:
		error = set_punctuation(s, current->value, current->adjust);
		break;
	case CMD_SET_RATE:
		error = set_rate(s, current->value, current->adjust);
		break;
	case CMD_SET_VOICE:
		error = EE_OK;
		break;
	case CMD_SET_VOLUME:
		error = set_volume(s, current->value, current->adjust);
		break;
	case CMD_SPEAK_TEXT:
		s->buf = current->buf;
		s->len = current->len;
		error = speak_text(s);
		break;
	case CMD_PAUSE:
		if (!paused_espeak) {
			espeak_Cancel();
			espeak_Terminate();
			paused_espeak = 1;
		}
		break;
	default:
		break;
	}

	if (error == EE_OK) {
		free_espeak_entry(current);
		current = NULL;
	}
}

int initialize_espeak(struct synth_t *s)
{
	int rate;

	/* initialize espeak */
	rate = espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 0, NULL, 0);
	if (rate < 0) {
		fprintf(stderr, "Unable to initialize espeak.\n");
		return -1;
	}

	espeak_SetSynthCallback(callback);

	/* Setup initial voice parameters */
	if (defaultVoice && defaultVoice[0]) {
		set_voice(s, defaultVoice);
		free(defaultVoice);
		defaultVoice = NULL;
	}
	set_frequency(s, defaultFrequency, ADJ_SET);
	set_pitch(s, defaultPitch, ADJ_SET);
	set_range(s, defaultRange, ADJ_SET);
	set_rate(s, defaultRate, ADJ_SET);
	set_volume(s, defaultVolume, ADJ_SET);
	espeak_SetParameter(espeakCAPITALS, 0, 0);
	paused_espeak = 0;
	return 0;
}

/* espeak_thread is the "main" function of our secondary (queue-processing)
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
void *espeak_thread(void *arg)
{
	struct synth_t *s = (struct synth_t *) arg;

	pthread_mutex_lock(&queue_guard);
	while (should_run) {

		while (should_run && !queue_peek(synth_queue) && !stop_requested)
			pthread_cond_wait(&runner_awake, &queue_guard);

		if (stop_requested) {
			stop_speech();
			synth_queue_clear();
			stop_requested = 0;
			pthread_cond_signal(&stop_acknowledged);
		}

		while (should_run && queue_peek(synth_queue) && !stop_requested) {
			queue_process_entry(s);
			pthread_mutex_lock(&queue_guard);
		}
	}
	pthread_cond_signal(&stop_acknowledged);
	pthread_mutex_unlock(&queue_guard);
	return NULL;
}
