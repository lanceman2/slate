SUBDIRS := pkgconfig

INSTALL_DIR = $(PREFIX)/lib


WAYLAND_CFLAGS = $(shell pkg-config wayland-client --cflags)
WAYLAND_LDFLAGS = $(shell pkg-config wayland-client --libs)


XDG_SHELL_PROTOCOL = $(shell pkg-config wayland-protocols\
 --variable=pkgdatadir)/stable/xdg-shell/xdg-shell.xml
WAYLAND_SCANNER = $(shell pkg-config --variable=wayland_scanner wayland-scanner)

xdg-shell-protocol.c:
	$(WAYLAND_SCANNER) private-code $(XDG_SHELL_PROTOCOL) xdg-shell-protocol.c

xdg-shell-client-protocol.h:
	$(WAYLAND_SCANNER) client-header $(XDG_SHELL_PROTOCOL) xdg-shell-client-protocol.h


BUILD_NO_INSTALL := xdg-shell-protocol.c xdg-shell-client-protocol.h

libslate.so_SOURCES :=\
 xdg-shell-protocol.c\
 debug.c\
 window.c
libslate.so_CPPFLAGS := $(WAYLAND_CFLAGS)
libslate.so_CFLAGS :=\
 -fvisibility=hidden\
 -DSL_EXPORT="__attribute__((visibility(\"default\"))) extern"

libslate.so_LDFLAGS :=\
 $(WAYLAND_LDFLAGS)\
 -Wl,--retain-symbols-file=retain-symbols.txt
libslate.so: retain-symbols.txt


include ../quickbuild.make
