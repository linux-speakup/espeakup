PREFIX = /usr/local
BINDIR = ${PREFIX}/bin
MANDIR = ${PREFIX}/share/man

DEPFLAGS = -MMD
WARNFLAGS = -Wall
CFLAGS += ${DEPFLAGS} ${WARNFLAGS}

LDLIBS = -lespeak -lpthread -lasound -lm

INSTALL = install
BINMODE = 0755
MANMODE = 0644
CHANGELOG_LIMIT?= --after="1 year ago"

SRCS = cli.c \
	espeak.c \
	espeakup.c  \
	queue.c \
	signal.c \
	softsynth.c \
	stringhandling.c

OBJS = ${SRCS:.c=.o}

all: espeakup

changelog:
	git log ${CHANGELOG_LIMIT} --format=full > ChangeLog

install: espeakup
	${INSTALL} -d ${DESTDIR}${BINDIR}
	${INSTALL} -m ${BINMODE} $< ${DESTDIR}${BINDIR}
	${INSTALL} -d ${DESTDIR}${MANDIR}/man8
	${INSTALL} -m ${MANMODE} espeakup.8 ${DESTDIR}${MANDIR}/man8

espeakup: ${OBJS}

clean:
	${RM} *.d *.o

distclean: clean
	${RM} espeakup

-include ${SRCS:.c=.d}
