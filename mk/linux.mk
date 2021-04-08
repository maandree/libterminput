LIBEXT      = so
LIBFLAGS    = -shared -Wl,-soname,libterminput.$(LIBEXT).$(LIB_MAJOR)
LIBMAJOREXT = $(LIBEXT).$(LIB_MAJOR)
LIBMINOREXT = $(LIBEXT).$(LIB_VERSION)
