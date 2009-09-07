INSTALL = install

PREFIX = /usr/local
MANDIR = $(PREFIX)/share/man/man8
BINDIR = $(PREFIX)/bin

ALSA_SRCS = alsa.c
PORTAUDIO_SRCS = portaudio.c
SRCS = \
	cli.c \
	espeak.c \
	espeakup.c  \
	queue.c \
	signal.c \
		softsynth.c

ifeq ($(AUDIO),alsa)
SRCS += $(ALSA_SRCS)
SOUNDLIB = -lasound
else
SRCS += $(PORTAUDIO_SRCS)
endif

OBJS = $(SRCS:.c=.o)

LDLIBS = -lespeak $(SOUNDLIB)

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

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -Wall -c $<

alsa.o: alsa.c espeakup.h

cli.o: cli.c espeakup.h

espeak.o: espeak.c espeakup.h queue.h

espeakup.o: espeakup.c espeakup.h queue.h

portaudio.o: portaudio.c espeakup.h

queue.o: queue.c queue.h

softsynth.o: softsynth.c espeakup.h queue.h

