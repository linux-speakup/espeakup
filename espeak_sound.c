#ifndef USE_ALSA
#include "espeakup.h"
int init_audio(void)
{
	audio_mode = AUDIO_OUTPUT_PLAYBACK;
	audio_callback = NULL;
	return 0;
}
#endif
