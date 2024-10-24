
# Get the libharfbuzz specific compiler options if we can.

libdir := $(shell pkg-config --variable=libdir harfbuzz)

# At run-time the environment variable LD_LIBRARY_PATH can override the
# DSO library file that it used at build-time by using the
# --enable-new-dtags linker option below:
#
HARFBUZZ_LDFLAGS := $(shell pkg-config --libs harfbuzz)\
 -Wl,--enable-new-dtags,-rpath,$(libdir)
HARFBUZZ_CFLAGS := $(shell pkg-config --cflags harfbuzz)

ifeq ($(libdir),)
# Use of harfbuzz is optional.
$(warning software package libharfbuzz was not found)
undefine HARFBUZZ_LDFLAGS
else
# Spew what libharfbuzz compiler options we have found
#$(info HARFBUZZ_CFLAGS="$(HARFBUZZ_CFLAGS)" HARFBUZZ_LDFLAGS="$(HARFBUZZ_LDFLAGS)")
endif

undefine libdir
