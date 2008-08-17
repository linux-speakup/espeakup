CC = cc
INSTALL = /usr/bin/install
RM = /bin/rm -f

SRCS = \
	espeakup.c  \
	queue.c \
		softsynth.c \
	synth.c

OBJS = $(SRCS:.c=.o)

LDLIBS = -lespeak

PREFIX = /usr
BINDIR = $(PREFIX)/bin

all: espeakup

install: espeakup
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $< $(DESTDIR)$(BINDIR)

clean:
	$(RM) $(OBJS)

distclean: clean
	$(RM) espeakup

espeakup: $(OBJS)

ifeq ("$(origin TAG)", "command line")
TIMESTAMP := _p$(shell date +%Y%m%d)
endif

VERSION := $(shell grep 'Version.*=' espeakup.c | sed 's/.*"\(.*\)";/\1/')
TAG := v$(VERSION)
TARPREFIX := espeakup-$(VERSION)
TARFILE := $(TARPREFIX)$(TIMESTAMP).tar

tarball:
	git archive --format=tar --prefix=$(TARPREFIX)/ $(TAG) > $(TARFILE)
	tar f $(TARFILE) --delete $(TARPREFIX)/.gitignore 
	bzip2 $(TARFILE)
