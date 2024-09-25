
# Get the libpixman-1 specific compiler options if we can.
PIXMAN_LDFLAGS := $(shell pkg-config --libs pixman-1)
PIXMAN_CFLAGS := $(shell pkg-config --cflags pixman-1)

ifeq ($(PIXMAN_LDFLAGS),)
# libpixman is optional
$(warning software package pixman was not found)
else
# Spew what pixman compiler options we have found
$(info PIXMAN_CFLAGS="$(PIXMAN_CFLAGS)" PIXMAN_LDFLAGS="$(PIXMAN_LDFLAGS)")
endif
