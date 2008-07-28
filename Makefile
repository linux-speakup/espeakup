LDLIBS = -lespeak

PROGRAM = espeakup

all: $(PROGRAM)

clean:
	rm $(PROGRAM)

install: $(PROGRAM)
	install -m 0755 $(PROGRAM) $(DESTDIR)/usr/bin

