INSTALL ?= install

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man

AUDIO ?= portaudio

alsa_SRCS = alsa.c
portaudio_SRCS = portaudio.c

SRCS = $($(AUDIO)_SRCS) \
	cli.c \
	espeak.c \
	espeakup.c  \
	queue.c \
	signal.c \
		softsynth.c
OBJS = $(SRCS:.c=.o)

DEPFLAGS = -MMD
WARNFLAGS = -Wall
ifeq ($(AUDIO),alsa)
SOUNDLIB = -lasound
endif
LDLIBS = -lespeak $(SOUNDLIB)

all: espeakup

install: espeakup
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)/man8
	$(INSTALL) -m 755 $< $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 644 espeakup.8 $(DESTDIR)$(MANDIR)/man8

espeakup: $(OBJS)

clean:
	$(RM) *.d *.o

distclean: clean
	$(RM) espeakup

%.o: %.c
	$(COMPILE.c) $(DEPFLAGS) $(WARNFLAGS) $(OUTPUT_OPTION) $<

-include $(SRCS:.c=.d)
