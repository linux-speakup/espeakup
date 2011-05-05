PREFIX = /usr/local
BINDIR = ${PREFIX}/bin
MANDIR = ${PREFIX}/share/man

DEPFLAGS = -MMD
WARNFLAGS = -Wall
CFLAGS += ${DEPFLAGS} ${WARNFLAGS}

LDLIBS = -lespeak -lpthread

INSTALL = install
BINMODE = 0755
MANMODE = 0644

SRCS = cli.c \
	espeak.c \
	espeakup.c  \
	queue.c \
	signal.c \
		softsynth.c

OBJS = ${SRCS:.c=.o}

all: espeakup

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
