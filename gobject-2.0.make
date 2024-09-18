
# Get the pangocairo specific compiler options if we can.
PANGOCAIRO_LDFLAGS := $(shell pkg-config --libs pangocairo)
PANGOCAIRO_CFLAGS := $(shell pkg-config --cflags pangocairo)

ifeq ($(PANGOCAIRO_LDFLAGS),)
$(warning software package pangocairo was not found)
else
# Spew what pangocairo compiler options we have found
$(info PANGOCAIRO_CFLAGS="$(PANGOCAIRO_CFLAGS)" PANGOCAIRO_LDFLAGS="$(PANGOCAIRO_LDFLAGS)")
endif
