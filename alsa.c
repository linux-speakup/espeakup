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

/*
 * File: alsa.c
 * Description: Produce audio by calling the ALSA library directly.
*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <alsa/asoundlib.h>

#include "espeakup.h"

static pthread_mutex_t audio_mutex = PTHREAD_MUTEX_INITIALIZER;

static snd_pcm_t *handle;

static void lock_audio_mutex(void)
{
	pthread_mutex_lock(&audio_mutex);
}

static void unlock_audio_mutex(void)
{
	pthread_mutex_unlock(&audio_mutex);
}

static int alsa_callback(short *audio, int numsamples, espeak_EVENT * events)
{
	int frames = 0;
	int rc = 0;

	lock_audio_mutex();
	if (stop_requested || !should_run) {
		unlock_audio_mutex();
		return 1;
	}

	while (numsamples > 0 && (!stop_requested && should_run)) {
		frames = snd_pcm_writei(handle, audio, numsamples);
		if (frames < 0)
			frames = snd_pcm_recover(handle, frames, !debug);
		if (frames < 0) {
			fprintf(stderr, "snd_pcm_writei failed: %s\n", snd_strerror(frames));
			break;
		}
		if (frames > 0 && frames < numsamples && debug)
			fprintf(stderr, "Short write (expected %i, wrote %i)\n", numsamples, frames);
		numsamples -= frames;
		audio += frames;
	}
	rc = (stop_requested || !should_run);
	unlock_audio_mutex();
	return rc;
}

void select_audio_mode(void)
{
	audio_mode = AUDIO_OUTPUT_RETRIEVAL;
}

int init_audio(unsigned int rate)
{
	int err;

	/* Open PCM device for playback. */
	err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		fprintf(stderr, "Playback open error: %s\n", snd_strerror(err));
		return err;
	}

	/* Set parameters. */
	err = snd_pcm_set_params(handle,
		SND_PCM_FORMAT_S16_LE,
		SND_PCM_ACCESS_RW_INTERLEAVED,
		1,
		rate,
		1,
		0);
	if (err < 0) {
		fprintf(stderr, "Playback open error: %s\n", snd_strerror(err));
		return err;
	}

	espeak_SetSynthCallback(alsa_callback);
	return 0;
}

void stop_audio(void)
{
	lock_audio_mutex();
	if (snd_pcm_drop(handle) < 0)
		fprintf(stderr, "Negative return from snd_pcm_drop!\n");
	snd_pcm_prepare(handle);
	unlock_audio_mutex();
}
