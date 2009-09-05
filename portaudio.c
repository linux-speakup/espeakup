#include "espeakup.h"

void select_audio_mode(void)
{
	audio_mode = AUDIO_OUTPUT_PLAYBACK;
}

int init_audio(unsigned int rate)
{
	return 0;
}

void stop_audio(void)
{
	return;
}
