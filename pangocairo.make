
# Get the gobject-2.0 specific compiler options if we can.
GOBJECT2_LDFLAGS := $(shell pkg-config --libs gobject-2.0)
GOBJECT2_CFLAGS := $(shell pkg-config --cflags gobject-2.0)

ifeq ($(GOBJECT2_LDFLAGS),)
$(warning software package gobject-2.0 was not found)
else
# Spew what gobject-2.0 compiler options we have found
$(info GOBJECT2_CFLAGS="$(GOBJECT2_CFLAGS)" GOBJECT2_LDFLAGS="$(GOBJECT2_LDFLAGS)")
endif
