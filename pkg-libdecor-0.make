
# Get the libdecor-0 specific compiler options if we can.

libdir := $(shell pkg-config --variable=libdir libdecor-0)

# At run-time the environment variable LD_LIBRARY_PATH can override the
# DSO library file that it used at build-time by using the
# --enable-new-dtags linker option below:
#
LIBDECOR_LDFLAGS := $(shell pkg-config --libs libdecor-0)\
 -Wl,--enable-new-dtags,-rpath,$(libdir)
LIBDECOR_CFLAGS := $(shell pkg-config --cflags libdecor-0)

ifeq ($(libdir),)
$(warning software package libdecor-0 was not found)
undefine LIBDECOR_LDFLAGS
else
# Spew what libdecor-0 compiler options we have found
#$(info LIBDECOR_CFLAGS="$(LIBDECOR_CFLAGS)" LIBDECOR_LDFLAGS="$(LIBDECOR_LDFLAGS)")
endif

undefine libdir
