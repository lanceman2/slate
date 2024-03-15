
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

_034_displayFailCleanup_SOURCES := _034_displayFailCleanup.c
_034_displayFailCleanup_LDFLAGS := $(SL_LIB)
_034_displayFailCleanup_CPPFLAGS := -DSLATE_NO_CLEANUP

035_displayCleanup_SOURCES := _034_displayFailCleanup.c
035_displayCleanup_LDFLAGS := $(SL_LIB)

_050_DSO.so_SOURCES := _050_DSO.c
_050_DSO.so_LDFLAGS := $(SL_LIB)

050_dlopen_SOURCES := 050_dlopen.c ../lib/debug.c





include ../quickbuild.make
