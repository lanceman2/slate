root := ..

include $(root)/pkg-freetype2.make
include $(root)/pkg-fontconfig.make
include $(root)/pkg-cairo.make
include $(root)/pkg-pangocairo.make
include $(root)/pkg-gobject-2.0.make
include $(root)/pkg-glib-2.0.make
include $(root)/pkg-harfbuzz.make
include $(root)/pkg-pixman-1.make



SL_LIB := -L../lib -lslate -Wl,-rpath=\$$ORIGIN/../lib


malloc_no_free_SOURCES := malloc_no_free.c ../lib/debug.c
004_malloc_free_SOURCES := malloc_free.c ../lib/debug.c


020_spew_SOURCES := 020_spew.c
020_spew_LDFLAGS := $(SL_LIB)
020_spew_CPPFLAGS := -I../include

030_display_SOURCES := 030_display.c
030_display_LDFLAGS := $(SL_LIB)

032_displayN_SOURCES := 032_displayN.c
032_displayN_LDFLAGS := $(SL_LIB)

displayFailCleanup_SOURCES := displayFailCleanup.c
displayFailCleanup_LDFLAGS := $(SL_LIB)
displayFailCleanup_CPPFLAGS := -DSLATE_NO_CLEANUP

035_displayCleanup_SOURCES := displayFailCleanup.c
035_displayCleanup_LDFLAGS := $(SL_LIB)

slateDSO.so_SOURCES := slateDSO.c
slateDSO.so_LDFLAGS := $(SL_LIB)

# This test doe not make a window, just a display.
050_dlopen_SOURCES := dlopen.c ../lib/debug.c
050_dlopen_CPPFLAGS := -DFUNC=\"makeDisplay\"

052_dlopen_SOURCES := dlopen.c ../lib/debug.c
052_dlopen_CPPFLAGS := -DFUNC=\"makeDisplayAndWindows\"

_060_window_SOURCES := _060_window.c
_060_window_LDFLAGS := $(SL_LIB)
_060_window_CPPFLAGS := -DCLEANUP

062_windows_SOURCES := 062_windows.c
062_windows_LDFLAGS := $(SL_LIB)

064_windows_SOURCES := 062_windows.c
064_windows_LDFLAGS := $(SL_LIB)
064_windows_CPPFLAGS := -DCLEANUP


_061_window_noCleanup_SOURCES := _060_window.c
_061_window_noCleanup_LDFLAGS := $(SL_LIB)

_066_showWindows_SOURCES := _066_showWindows.c
_066_showWindows_LDFLAGS := $(SL_LIB)


windowCountDraw_SOURCES := windowCountDraw.c
windowCountDraw_LDFLAGS := $(SL_LIB)

draw_SOURCES := draw.c
draw_LDFLAGS := $(SL_LIB)

068_drawFunc_SOURCES := 068_drawFunc.c
068_drawFunc_LDFLAGS := $(SL_LIB)



070_popup_SOURCES := 070_popup.c
070_popup_LDFLAGS := $(SL_LIB)

DrawPopup_SOURCES := DrawPopup.c
DrawPopup_LDFLAGS := $(SL_LIB)


102_helloFreeType_SOURCES := 102_helloFreeType.c
102_helloFreeType_LDFLAGS := $(FREETYPE2_LDFLAGS) $(SL_LIB) -lm
102_helloFreeType_CPPFLAGS := $(FREETYPE2_CFLAGS)

helloFreeType_SOURCES := 102_helloFreeType.c
helloFreeType_LDFLAGS := $(FREETYPE2_LDFLAGS) $(SL_LIB) -lm
helloFreeType_CPPFLAGS := $(FREETYPE2_CFLAGS) -D LOOP


111_fontconfig_SOURCES := 111_fontconfig.c ../lib/debug.c
111_fontconfig_LDFLAGS := $(FONTCONFIG_LDFLAGS)
111_fontconfig_CPPFLAGS := $(FONTCONFIG_CFLAGS)

113_FindFont_SOURCES := 113_FindFont.c
113_FindFont_LDFLAGS := $(SL_LIB)

130_font_SOURCES := 130_font.c
130_font_LDFLAGS := $(SL_LIB)

font_SOURCES := 130_font.c
font_LDFLAGS := $(SL_LIB)
font_CPPFLAGS := -D LOOP

cairoDraw_SOURCES := cairoDraw.c
cairoDraw_LDFLAGS := $(SL_LIB) $(CAIRO_LDFLAGS)
cairoDraw_CPPFLAGS := $(CAIRO_CFLAGS)


ifneq ($(PANGOCAIRO_LDFLAGS),)
pangocairo_SOURCES := pangocairo.c ../lib/debug.c
pangocairo_LDFLAGS := $(PANGOCAIRO_LDFLAGS) -lm
pangocairo_CPPFLAGS := $(PANGOCAIRO_CFLAGS)
endif

ifneq ($(GOBJECT2_LDFLAGS),)
gobject2_SOURCES := gobject2.c
gobject2_LDFLAGS := $(GOBJECT2_LDFLAGS)
gobject2_CPPFLAGS := $(GOBJECT2_CFLAGS)
endif

ifneq ($(PIXMAN_LDFLAGS),)
pixman-1_SOURCES := pixman-1.c
pixman-1_LDFLAGS := $(PIXMAN_LDFLAGS)
pixman-1_CPPFLAGS := $(PIXMAN_CFLAGS)
endif

ifneq ($(GLIB2_LDFLAGS),)
glib2_SOURCES := glib2.c
glib2_LDFLAGS := $(GLIB2_LDFLAGS)
glib2_CPPFLAGS := $(GLIB2_CFLAGS)
endif

ifneq ($(HARFBUZZ_LDFLAGS),)
harfbuzz_SOURCES := harfbuzz.c
harfbuzz_LDFLAGS := $(HARFBUZZ_LDFLAGS)
harfbuzz_CPPFLAGS := $(HARFBUZZ_CFLAGS)
endif



test: build
	./run_tests


include ../quickbuild.make

