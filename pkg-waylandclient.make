
# Get the wayland client specific compiler options if we can.

libdir := $(shell pkg-config --variable=libdir wayland-client)

# At run-time the environment variable LD_LIBRARY_PATH can override the
# DSO library file that it used at build-time by using the
# --enable-new-dtags linker option below:
#
WL_LDFLAGS := $(shell pkg-config --libs wayland-client)\
 -Wl,--enable-new-dtags,-rpath,$(libdir)
WL_CFLAGS := $(shell pkg-config --cflags wayland-client)


ifeq ($(libdir),)
# Use of wayland-client is required.
$(error software package wayland-client was not found)
endif

# Spew what wayland client compiler options we have found
$(info WL_CFLAGS="$(WL_CFLAGS)" WL_LDFLAGS="$(WL_LDFLAGS)")

# TODO: Do these make lines add other failure modes?:
WL_PROTOCOL := $(shell pkg-config wayland-protocols\
 --variable=pkgdatadir)/stable/xdg-shell/xdg-shell.xml
WL_SCANNER := $(shell pkg-config --variable=wayland_scanner wayland-scanner)

undefine libdir

# TODO: Bug in wayland-protocols pkgdatadir adds extra "/"
# like: //usr/share/wayland-protocols  Mar 15 2024

