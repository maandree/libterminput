.POSIX:

LIB_MAJOR = 1
LIB_MINOR = 0
LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR)


CONFIGFILE = config.mk
include $(CONFIGFILE)

OS = linux
# Linux:   linux
# Mac OS:  macos
# Windows: windows 
include mk/$(OS).mk



LOBJ =\
	libterminput.lo

HDR =\
	libterminput.h

OBJ =\
	interactive-test.o\
	$(LOBJ:.lo=.o)


all: libterminput.a libterminput.$(LIBEXT) interactive-test
$(OBJ): $(@:.o=.c) $(HDR)
$(LOBJ): $(@:.lo=.c) $(HDR)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

.c.lo:
	$(CC) -fPIC -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

.o.a:
	-rm -f -- $@
	$(AR) rc $@ $<
	$(AR) -s $@

.lo.$(LIBEXT):
	$(CC) $(LIBFLAGS) $(LDFLAGS_METHODS) -o $@ $< $(LDFLAGS)

interactive-test: interactive-test.o libterminput.a
	$(CC) -o $@ interactive-test.o libterminput.a $(LDFLAGS)

install: libterminput.a libterminput.$(LIBEXT)
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man3"
	cp -- libterminput.a "$(DESTDIR)$(PREFIX)/lib/"
	cp -- libterminput.h "$(DESTDIR)$(PREFIX)/include/"
	cp -- libterminput.$(LIBEXT) "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBMINOREXT)"
	ln -sf -- libterminput.$(LIBMINOREXT) "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBMAJOREXT)"
	ln -sf -- libterminput.$(LIBMAJOREXT) "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBEXT)"
	cp -- libterminput_set_flags.3 "$(DESTDIR)$(MANPREFIX)/man3"
	ln -sf -- libterminput_set_flags.3 "$(DESTDIR)$(MANPREFIX)/man3/libterminput_clear_flags.3"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBMAJOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBMINOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBEXT)"
	-rm -rf -- "$(DESTDIR)$(PREFIX)/lib/libterminput.a"
	-rm -rf -- "$(DESTDIR)$(PREFIX)/include/libterminput.h"
	-rm -rf -- "$(DESTDIR)$(MANPREFIX)/man3/libterminput_set_flags.3"
	-rm -rf -- "$(DESTDIR)$(MANPREFIX)/man3/libterminput_clear_flags.3"

clean:
	-rm -f -- *.o *.a *.lo *.so *.su *.dll *.dylib interactive-test

.SUFFIXES:
.SUFFIXES: .a .o .lo .c .$(LIBEXT)

.PHONY: all install uninstall clean
