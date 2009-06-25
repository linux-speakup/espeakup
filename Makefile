CFLAGS += -Wall
LDLIBS = -lespeak

PREFIX = /usr
MANDIR = $(PREFIX)/share/man/man8
BINDIR = $(PREFIX)/bin

INSTALL = install

ALSA_SRCS = alsa.c
ESPEAK_SRCS = espeak_sound.c
SRCS = \
	cli.c \
	espeakup.c  \
	queue.c \
	signal.c \
		softsynth.c \
	synth.c

ifeq ($(AUDIO),alsa)
SRCS += $(ALSA_SRCS)
LDLIBS += -lasound
else
SRCS += $(ESPEAK_SRCS)
endif

OBJS = $(SRCS:.c=.o)

all: espeakup

install: espeakup
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $< $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)
	$(INSTALL) -m 0644 espeakup.8 $(DESTDIR)$(MANDIR)

clean:
	$(RM) *.o

distclean: clean
	$(RM) espeakup

espeakup: $(OBJS)

cli.o: cli.c espeakup.h

espeakup.o: espeakup.c espeakup.h

queue.o: queue.c espeakup.h

softsynth.o: softsynth.c espeakup.h

synth.o: synth.c espeakup.h

alsa.o: alsa.c espeakup.h

espeak_sound.o: espeak_sound.c espeakup.h
