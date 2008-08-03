CC = cc
INSTALL = /usr/bin/install
RM = /bin/rm

PREFIX = /usr
BINDIR = $(PREFIX)/bin

LDLIBS = -lespeak

all: espeakup

install: espeakup
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $< $(DESTDIR)$(BINDIR)

clean:
	$(RM) espeakup

