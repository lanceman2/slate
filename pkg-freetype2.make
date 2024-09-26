
# Get the freetype2 specific compiler options if we can.

libdir := $(shell pkg-config --variable=libdir freetype2)

# At run-time the environment variable LD_LIBRARY_PATH can override the
# DSO library file that it used at build-time by using the
# --enable-new-dtags linker option below:
#
FREETYPE2_LDFLAGS := $(shell pkg-config --libs freetype2)\
 -Wl,--enable-new-dtags,-rpath,$(libdir)
FREETYPE2_CFLAGS := $(shell pkg-config --cflags freetype2)

ifeq ($(libdir),)
$(error software package freetype2 was not found)
endif

# Spew what freetype2 compiler options we have found
$(info FREETYPE2_CFLAGS="$(FREETYPE2_CFLAGS)" FREETYPE2_LDFLAGS="$(FREETYPE2_LDFLAGS)")

undefine libdir
