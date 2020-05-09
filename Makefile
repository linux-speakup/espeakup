PREFIX = /usr/local
BINDIR = ${PREFIX}/bin
MANDIR = ${PREFIX}/share/man
SRC_DIR = src
DEPFLAGS = -MMD
WARNFLAGS = -Wall -Wpedantic \
            -Wextra

CFLAGS += ${DEPFLAGS} ${WARNFLAGS}

LDLIBS = -lespeak-ng -lpthread -lasound -lm

INSTALL = install
BINMODE = 0755
MANMODE = 0644

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst %.c,%.o,$(wildcard $(SRC_DIR)/*.c))

all: espeakup

install: espeakup
	${INSTALL} -d ${DESTDIR}${BINDIR}
	${INSTALL} -m ${BINMODE} $< ${DESTDIR}${BINDIR}
	${INSTALL} -d ${DESTDIR}${MANDIR}/man8
	${INSTALL} -m ${MANMODE} doc/espeakup.8 ${DESTDIR}${MANDIR}/man8

espeakup: ${OBJECTS}
	cc $(LDLIBS) -o ./espeakup $(OBJECTS)

clean:
	${RM} $(SRC_DIR)/*.d $(SRC_DIR)/*.o

distclean: clean
	${RM} espeakup

-include ${SRCS:.c=.d}
