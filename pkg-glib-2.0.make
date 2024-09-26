
# Get the libglib-2.0 specific compiler options if we can.

libdir := $(shell pkg-config --variable=libdir glib-2.0)

# At run-time the environment variable LD_LIBRARY_PATH can override the
# DSO library file that it used at build-time by using the
# --enable-new-dtags linker option below:
#
GLIB2_LDFLAGS := $(shell pkg-config --libs glib-2.0)\
 -Wl,--enable-new-dtags,-rpath,$(libdir)
GLIB2_CFLAGS := $(shell pkg-config --cflags glib-2.0)

ifeq ($(libdir),)
# Use of libglib-2.0 is optional.
$(warning software package libglib-2.0 was not found)
undefine GLIB2_LDFLAGS
else
# Spew what libglib-2.0 compiler options we have found
$(info GLIB2_CFLAGS="$(GLIB2_CFLAGS)" GLIB2_LDFLAGS="$(GLIB2_LDFLAGS)")
endif

undefine libdir
