
# Get the libharfbuzz specific compiler options if we can.
HARFBUZZ_LDFLAGS := $(shell pkg-config --libs harfbuzz)
HARFBUZZ_CFLAGS := $(shell pkg-config --cflags harfbuzz)


ifeq ($(HARFBUZZ_LDFLAGS),)
# harfbuzz is optional
$(warning software package libharfbuzz was not found)
else
# Spew what libharfbuzz compiler options we have found
$(info HARFBUZZ_CFLAGS="$(HARFBUZZ_CFLAGS)" HARFBUZZ_LDFLAGS="$(HARFBUZZ_LDFLAGS)")
endif

