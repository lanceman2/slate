SUBDIRS := pkgconfig

INSTALL_DIR = $(PREFIX)/lib


WAYLAND_FLAGS = $(shell pkg-config wayland-client --cflags --libs)
XDG_SHELL_PROTOCOL = $(shell pkg-config wayland-protocols --variable=pkgdatadir)/stable/xdg-shell/xdg-shell.xml


WAYLAND_SCANNER = $(shell pkg-config --variable=wayland_scanner wayland-scanner)

xdg-shell-protocol.c:
	$(WAYLAND_SCANNER) private-code $(XDG_SHELL_PROTOCOL) xdg-shell-protocol.c

xdg-shell-client-protocol.h:
	$(WAYLAND_SCANNER) client-header $(XDG_SHELL_PROTOCOL) xdg-shell-client-protocol.h



BUILD_NO_INSTALL := xdg-shell-protocol.c xdg-shell-client-protocol.h




include ../quickbuild.make
