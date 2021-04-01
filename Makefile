.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)


OBJ =\
	interactive-test.o\
	libterminput.o

HDR =\
	libterminput.h


all: libterminput.a interactive-test
$(OBJ): $(@:.o=.c) $(HDR)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

.o.a:
	$(AR) rc $@ $<
	$(AR) -s $@

interactive-test: interactive-test.o libterminput.a
	$(CC) -o $@ interactive-test.o libterminput.a $(LDFLAGS)

install: libterminput.a
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	cp -- libterminput.a "$(DESTDIR)$(PREFIX)/lib/"
	cp -- libterminput.h "$(DESTDIR)$(PREFIX)/include/"

uninstall:
	rm -- "$(DESTDIR)$(PREFIX)/lib/libterminput.a"
	rm -- "$(DESTDIR)$(PREFIX)/include/libterminput.h"

clean:
	-rm -f -- *.o *.a interactive-test

.SUFFIXES:
.SUFFIXES: .a .o .c

.PHONY: all install uninstall clean
