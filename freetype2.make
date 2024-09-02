
# Get the freetype2 specific compiler options if we can.
FREETYPE2_LDFLAGS := $(shell pkg-config --libs freetype2)
FREETYPE2_CFLAGS := $(shell pkg-config --cflags freetype2)

ifeq ($(FREETYPE2_LDFLAGS),)
$(error software package freetype2 was not found)
else
# Spew what freetype2 compiler options we have found
$(info FREETYPE2_CFLAGS="$(FREETYPE2_CFLAGS)" FREETYPE2_LDFLAGS="$(FREETYPE2_LDFLAGS)")
endif
