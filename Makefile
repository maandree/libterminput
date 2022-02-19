.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

OS = linux
# Linux:   linux
# Mac OS:  macos
# Windows: windows 
include mk/$(OS).mk


LIB_MAJOR = 1
LIB_MINOR = 0
LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR)


OBJ =\
	libterminput.o

HDR =\
	libterminput.h

TESTS =\
	interactive-test\
	test

LOBJ = $(OBJ:.o=.lo)


all: libterminput.a libterminput.$(LIBEXT) $(TESTS)
$(OBJ): $(HDR)
$(LOBJ): $(HDR)
$(TESTS:=.o): $(HDR)

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

.c.lo:
	$(CC) -fPIC -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

interactive-test: interactive-test.o libterminput.a
	$(CC) -o $@ interactive-test.o libterminput.a $(LDFLAGS)

test: test.o libterminput.a
	$(CC) -o $@ test.o libterminput.a $(LDFLAGS)

libterminput.$(LIBEXT): $(LOBJ)
	$(CC) $(LIBFLAGS) -o $@ $(LOBJ) $(LDFLAGS)

libterminput.a: $(OBJ)
	-rm -f -- $@
	$(AR) rc $@ $(OBJ)
	$(AR) -s $@

check: test
	./test

install: libterminput.a libterminput.$(LIBEXT)
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man3"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man7"
	cp -- libterminput.a "$(DESTDIR)$(PREFIX)/lib/"
	cp -- libterminput.h "$(DESTDIR)$(PREFIX)/include/"
	cp -- libterminput.$(LIBEXT) "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBMINOREXT)"
	$(FIX_INSTALL_NAME) "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBMINOREXT)"
	ln -sf -- libterminput.$(LIBMINOREXT) "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBMAJOREXT)"
	ln -sf -- libterminput.$(LIBMAJOREXT) "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBEXT)"
	cp -- libterminput_read.3 libterminput_set_flags.3 libterminput_is_ready.3 "$(DESTDIR)$(MANPREFIX)/man3"
	ln -sf -- libterminput_set_flags.3 "$(DESTDIR)$(MANPREFIX)/man3/libterminput_clear_flags.3"
	cp -- libterminput.7 "$(DESTDIR)$(MANPREFIX)/man7"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBMAJOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBMINOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libterminput.$(LIBEXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libterminput.a"
	-rm -f -- "$(DESTDIR)$(PREFIX)/include/libterminput.h"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man3/libterminput_read.3"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man3/libterminput_set_flags.3"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man3/libterminput_clear_flags.3"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man3/libterminput_is_ready.3"
	-rm -f -- "$(DESTDIR)$(MANPREFIX)/man7/libterminput.7"

clean:
	-rm -f -- *.o *.a *.lo *.so *.so.* *.su *.dll *.dylib interactive-test test

.SUFFIXES:
.SUFFIXES: .lo .o .c

.PHONY: all check install uninstall clean
