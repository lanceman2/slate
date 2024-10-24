SUBDIRS := pkgconfig

INSTALL_DIR = $(PREFIX)/lib

root := ..

include $(root)/pkg-freetype2.make
include $(root)/pkg-fontconfig.make
include $(root)/pkg-waylandclient.make
include $(root)/pkg-cairo.make
include $(root)/pkg-pixman-1.make


xdg-shell-protocol.c:
	$(WL_SCANNER) private-code $(WL_PROTOCOL) $@
xdg-shell-protocol.h:
	$(WL_SCANNER) client-header $(WL_PROTOCOL) $@

xdg_decoration_xml := unstable/xdg-decoration/xdg-decoration-unstable-v1.xml

xdg-decoration-protocol.c:
	$(WL_SCANNER) private-code $(WL_PROTOCOL_DIR)/$(xdg_decoration_xml) $@
xdg-decoration-protocol.h:
	$(WL_SCANNER) client-header $(WL_PROTOCOL_DIR)/$(xdg_decoration_xml) $@

# We need to build xdg-shell-client-protocol.h before we create depend
# files (we made edits to ../quickbuild.make for this case):
PRE_BUILD :=\
 xdg-shell-protocol.h\
 xdg-decoration-protocol.h

BUILD_NO_INSTALL :=\
 xdg-shell-protocol.c\
 xdg-decoration-protocol.c

libslate.so_SOURCES :=\
 xdg-shell-protocol.c\
 xdg-decoration-protocol.c\
 debug.c\
 display.c\
 window.c\
 slWindow_show.c\
 popup.c\
 drawingUtils.c\
 cairoDraw.c\
 findFont.c\
 font.c\
 shm.c\
 widget.c
libslate.so_CPPFLAGS :=\
 $(WL_CFLAGS) $(CAIRO_CFLAGS) $(FONTCONFIG_CFLAGS)\
 -DSLATE_LIB_CODE
libslate.so_CFLAGS :=\
 -fvisibility=hidden\
 -DSL_EXPORT="__attribute__((visibility(\"default\"))) extern"\
 -DEXPORT="SL_EXPORT"
# good read:
#   https://amir.rachum.com/shared-libraries/
#
# The order of these LDFLAGS can matter.  We put CAIRO_LDFLAGS before
# FONTCONFIG_LDFLAGS because we needed the libcairo.so to be from a
# version we compiled without X11 stuff.  FONTCONFIG_LDFLAGS added a
# search path that included a (system) libcairo.so version that was
# compiled with X11 client libraries.  This way may not work for other
# cases.  At some put we need an operating system package manager that
# fixes this kind of (DSO dependency hell) shit, like NixOS.
#
# Run in a shell:
#
#   readelf -d libslate.so
#
#
# It would appear there is no order that works for the way all the
# libraries can be installed; for all users.  We added
# $(PIXMAN_LDFLAGS) because we wanted to use a our patched version
# of libpixman-1.so.
#
libslate.so_LDFLAGS :=\
 $(PIXMAN_LDFLAGS)\
 $(CAIRO_LDFLAGS)\
 $(WL_LDFLAGS)\
 $(FONTCONFIG_LDFLAGS)\
 -Wl,--retain-symbols-file=retain-symbols.txt
libslate.so: retain-symbols.txt


include ../quickbuild.make

