CC = cc
INSTALL = /usr/bin/install
RM = /bin/rm -f

SRCS = \
	cli.c \
	espeakup.c  \
	queue.c \
		softsynth.c \
	synth.c

OBJS = $(SRCS:.c=.o)

LDLIBS = -lespeak

PREFIX = /usr
BINDIR = $(PREFIX)/bin

ifeq ("$(origin TAG)", "command line")
TIMESTAMP := _p$(shell date +%Y%m%d)
endif

VERSION := $(shell grep 'Version.*=' cli.c | sed 's/.*"\(.*\)";/\1/')
TAG := v$(VERSION)
TARPREFIX := espeakup-$(VERSION)
TARFILE := $(TARPREFIX)$(TIMESTAMP).tar

all: espeakup

install: espeakup
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $< $(DESTDIR)$(BINDIR)

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) espeakup

espeakup: $(OBJS)

tarball:
	git archive --format=tar --prefix=$(TARPREFIX)/ $(TAG) > $(TARFILE)
	tar f $(TARFILE) --delete $(TARPREFIX)/.gitignore 
	bzip2 $(TARFILE)

cli.o: cli.c espeakup.h

espeakup.o: espeakup.c espeakup.h

queue.o: queue.c espeakup.h

softsynth.o: softsynth.c espeakup.h

synth.o: synth.c espeakup.h
