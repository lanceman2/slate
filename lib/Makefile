SUBDIRS := pkgconfig

INSTALL_DIR = $(PREFIX)/lib

root := ..

include $(root)/freetype2.make
include $(root)/fontconfig.make
include $(root)/waylandclient.make


xdg-shell-protocol.c:
	$(WL_SCANNER) private-code $(WL_PROTOCOL) xdg-shell-protocol.c

xdg-shell-client-protocol.h:
	$(WL_SCANNER) client-header $(WL_PROTOCOL)\
 xdg-shell-client-protocol.h

# We need to build xdg-shell-client-protocol.h before we create depend
# files:
PRE_BUILD := xdg-shell-client-protocol.h
BUILD_NO_INSTALL := xdg-shell-protocol.c

libslate.so_SOURCES :=\
 xdg-shell-protocol.c\
 debug.c\
 display.c\
 window.c\
 popup.c\
 shm.c
libslate.so_CPPFLAGS := $(WL_CFLAGS)
libslate.so_CFLAGS :=\
 -fvisibility=hidden\
 -DSL_EXPORT="__attribute__((visibility(\"default\"))) extern"\
 -DEXPORT="SL_EXPORT"
libslate.so_LDFLAGS :=\
 $(WL_LDFLAGS)\
 -Wl,--retain-symbols-file=retain-symbols.txt
libslate.so: retain-symbols.txt


include ../quickbuild.make

