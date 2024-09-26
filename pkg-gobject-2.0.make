
# Get the gobject-2.0 specific compiler options if we can.

libdir := $(shell pkg-config --variable=libdir gobject-2.0)

# At run-time the environment variable LD_LIBRARY_PATH can override the
# DSO library file that it used at build-time by using the
# --enable-new-dtags linker option below:
#
GOBJECT2_LDFLAGS := $(shell pkg-config --libs gobject-2.0)\
 -Wl,--enable-new-dtags,-rpath,$(libdir)
GOBJECT2_CFLAGS := $(shell pkg-config --cflags gobject-2.0)

ifeq ($(libdir),)
$(warning software package gobject-2.0 was not found)
undefine GOBJECT2_LDFLAGS
else
# Spew what gobject-2.0 compiler options we have found
$(info GOBJECT2_CFLAGS="$(GOBJECT2_CFLAGS)" GOBJECT2_LDFLAGS="$(GOBJECT2_LDFLAGS)")
endif

undefine libdir
