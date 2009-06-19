INSTALL = install

CFLAGS ?= -DUSE_ALSA
SRCS = \
	cli.c \
	alsa.c \
	espeakup.c  \
	espeak_sound.c \
	queue.c \
		softsynth.c \
	synth.c

OBJS = $(SRCS:.c=.o)

LDLIBS = -lespeak -lasound

PREFIX = /usr
MANDIR = $(PREFIX)/share/man/man8
BINDIR = $(PREFIX)/bin

all: espeakup

install: espeakup
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $< $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)
	$(INSTALL) -m 0644 espeakup.8 $(DESTDIR)$(MANDIR)

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) espeakup

espeakup: $(OBJS)

cli.o: cli.c espeakup.h

espeakup.o: espeakup.c espeakup.h

queue.o: queue.c espeakup.h

softsynth.o: softsynth.c espeakup.h

synth.o: synth.c espeakup.h

%.o: %.c
	$(CC) -c -Wall $(CFLAGS) $(CPPFLAGS) -o $@ $< 
