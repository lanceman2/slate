
# Get the pangocairo specific compiler options if we can.

libdir := $(shell pkg-config --variable=libdir pangocairo)

# At run-time the environment variable LD_LIBRARY_PATH can override the
# DSO library file that it used at build-time by using the
# --enable-new-dtags linker option below:
#
PANGOCAIRO_LDFLAGS := $(shell pkg-config --libs pangocairo)\
 -Wl,--enable-new-dtags,-rpath,$(libdir)
PANGOCAIRO_CFLAGS := $(shell pkg-config --cflags pangocairo)

ifeq ($(libdir),)
# Use of pangocairo is optional
$(warning software package pangocairo was not found)
undefine PANGOCAIRO_LDFLAGS
else
# Spew what pangocairo compiler options we have found
#$(info PANGOCAIRO_CFLAGS="$(PANGOCAIRO_CFLAGS)" PANGOCAIRO_LDFLAGS="$(PANGOCAIRO_LDFLAGS)")
endif

undefine libdir 
