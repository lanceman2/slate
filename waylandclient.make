
# Get the wayland client specific compiler options if we can.
WL_LDFLAGS := $(shell pkg-config --libs wayland-client)
WL_CFLAGS := $(shell pkg-config --cflags wayland-client)


ifeq ($(WL_LDFLAGS),)
$(error software package wayland-client was not found)
else
# Spew what wayland client compiler options we have found
$(info WL_CFLAGS="$(WL_CFLAGS)" WL_LDFLAGS="$(WL_LDFLAGS)")
endif


# TODO: Do these make lines add other failure modes:
WL_PROTOCOL := $(shell pkg-config wayland-protocols\
 --variable=pkgdatadir)/stable/xdg-shell/xdg-shell.xml
WL_SCANNER := $(shell pkg-config --variable=wayland_scanner wayland-scanner)

# TODO: Bug in wayland-protocols pkgdatadir adds extra "/"
# like: //usr/share/wayland-protocols  Mar 15 2024

