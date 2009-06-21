#include "espeakup.h"
int init_audio(void)
{
	audio_mode = AUDIO_OUTPUT_PLAYBACK;
	audio_callback = NULL;
	return 0;
}

/*
 * lock_audio_mutex and unlock_audio_mutex are no-ops if we use native
 * sound support.  The stopped variable is never read; no need to protect it.
 */

void lock_audio_mutex(void)
{
	return;
}

void unlock_audio_mutex(void)
{
	return;
}
