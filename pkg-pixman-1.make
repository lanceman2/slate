# Access to libpixman-1.so may have been gotten indirectly through
# libcairo.so which depends on libpixman-1.so, but we do not require
# that the development files for pixman-1 be installed.  We only use
# the pixman-1 development files for testing/developing, they are
# optional.

libdir := $(shell pkg-config --variable=libdir pixman-1)

# Get the libpixman-1 specific compiler options if we can.
PIXMAN_LDFLAGS := $(shell pkg-config --libs pixman-1)\
 -Wl,--enable-new-dtags,-rpath,$(libdir)
PIXMAN_CFLAGS := $(shell pkg-config --cflags pixman-1)

ifeq ($(libdir),)
# Use of libpixman is optional
$(warning software package pixman-1 was not found)
undefine PIXMAN_LDFLAGS
else
# Spew what pixman compiler options we have found
$(info PIXMAN_CFLAGS="$(PIXMAN_CFLAGS)" PIXMAN_LDFLAGS="$(PIXMAN_LDFLAGS)")
endif

undefine libdir
