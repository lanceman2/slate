
# Get the libdecor-0 specific compiler options.
#
# libdecor provides client-side window decorations for Wayland clients.
# Since the GNOME wayland server, "mutter", does not support server side
# window decorations (via zxdg_decoration_manager and stuff), libdecor
# provides client-side window decorations.  See noticed that the
# libdecor-0 does not have the library bloat like most other GNOME
# libraries; very few dependences; see for yourself with:
#
#   ldd $(pkg-config --variable=libdir libdecor-0)/libdecor-0.so
#
#
# In the libslate.so API we provide slDisplay_haveXDGDecoration() to
# test if there is a zxdg_decoration_manager (stuff) on the running
# system.
#

libdir := $(shell pkg-config --variable=libdir libdecor-0)

# At run-time the environment variable LD_LIBRARY_PATH can override the
# DSO library file that it used at build-time by using the
# --enable-new-dtags linker option below:
#
LIBDECOR_LDFLAGS := $(shell pkg-config --libs libdecor-0)\
 -Wl,--enable-new-dtags,-rpath,$(libdir)
LIBDECOR_CFLAGS := $(shell pkg-config --cflags libdecor-0)

ifeq ($(libdir),)
$(error software package libdecor-0 was not found)
undefine LIBDECOR_LDFLAGS
else
# Spew what libdecor-0 compiler options we have found
#$(info LIBDECOR_CFLAGS="$(LIBDECOR_CFLAGS)" LIBDECOR_LDFLAGS="$(LIBDECOR_LDFLAGS)")
endif

undefine libdir
