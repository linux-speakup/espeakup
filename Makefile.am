bin_PROGRAMS = espeakup
espeakup_SOURCES = espeakup.c espeakup.h cli.c espeak.c queue.c queue.h signal.c softsynth.c

if ALSA
espeakup_SOURCES += alsa.c
else
espeakup_SOURCES += portaudio.c
endif

if STANDALONE
espeakup_LDFLAGS = -all-static
endif

dist_man8_MANS = espeakup.8
