
# Get the libglib-2.0 specific compiler options if we can.
GLIB2_LDFLAGS := $(shell pkg-config --libs glib-2.0)
GLIB2_CFLAGS := $(shell pkg-config --cflags glib-2.0)

ifeq ($(GLIB2_LDFLAGS),)
$(warning software package libglib-2.0 was not found)
else
# Spew what libglib-2.0 compiler options we have found
$(info GLIB2_CFLAGS="$(GLIB2_CFLAGS)" GLIB2_LDFLAGS="$(GLIB2_LDFLAGS)")
endif
