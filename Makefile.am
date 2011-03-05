bin_PROGRAMS = espeakup
espeakup_SOURCES = espeakup.c espeakup.h cli.c espeak.c queue.c queue.h signal.c softsynth.c

if STANDALONE
espeakup_LDFLAGS = -all-static
endif

dist_man8_MANS = espeakup.8
