
# Get the cairo specific compiler options if we can.
CAIRO_LDFLAGS := $(shell pkg-config --libs cairo)
CAIRO_CFLAGS := $(shell pkg-config --cflags cairo)

ifeq ($(CAIRO_LDFLAGS),)
$(error software package cairo was not found)
else
# Spew what cairo compiler options we have found
$(info CAIRO_CFLAGS="$(CAIRO_CFLAGS)" CAIRO_LDFLAGS="$(CAIRO_LDFLAGS)")
endif
