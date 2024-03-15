SUBDIRS := pkgconfig

INSTALL_DIR = $(PREFIX)/lib


WL_CFLAGS := $(shell pkg-config wayland-client --cflags)
WL_LDFLAGS := $(shell pkg-config wayland-client --libs)


ifeq ($(WL_LDFLAGS),)
# Make make fail if pkg-config cannot find wayland-client.
#
# TODO: Are there other common wayland-client failure modes that can
# happen when pkg-config can find wayland-client?  Like with the
# wayland_scanner?
#
  $(error pkg-config wayland-client --libs FAILED)
endif

# TODO: Bug in wayland-protocols pkgdatadir adds extra "/"
# like: //usr/share/wayland-protocols  Mar 15 2024
#
WL_PROTOCOL := $(shell pkg-config wayland-protocols\
 --variable=pkgdatadir)/stable/xdg-shell/xdg-shell.xml
WL_SCANNER := $(shell pkg-config --variable=wayland_scanner wayland-scanner)

xdg-shell-protocol.c:
	$(WL_SCANNER) private-code $(WL_PROTOCOL) xdg-shell-protocol.c

xdg-shell-client-protocol.h:
	$(WL_SCANNER) client-header $(WL_PROTOCOL)\
 xdg-shell-client-protocol.h


BUILD_NO_INSTALL := xdg-shell-protocol.c xdg-shell-client-protocol.h

libslate.so_SOURCES :=\
 xdg-shell-protocol.c\
 debug.c\
 display.c\
 window.c
libslate.so_CPPFLAGS := $(WL_CFLAGS)
libslate.so_CFLAGS :=\
 -fvisibility=hidden\
 -DSL_EXPORT="__attribute__((visibility(\"default\"))) extern"
libslate.so_LDFLAGS :=\
 $(WL_LDFLAGS)\
 -Wl,--retain-symbols-file=retain-symbols.txt
libslate.so: retain-symbols.txt


include ../quickbuild.make

