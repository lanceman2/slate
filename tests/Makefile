
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

050_dlopen_SOURCES := 050_dlopen.c ../lib/debug.c

_060_window_SOURCES := _060_window.c
_060_window_LDFLAGS := $(SL_LIB)
_060_window_CPPFLAGS := -DCLEANUP

062_windows_SOURCES := 062_windows.c
062_windows_LDFLAGS := $(SL_LIB)
062_windows_CPPFLAGS := -DCLEANUP


_061_window_noCleanup_SOURCES := _060_window.c
_061_window_noCleanup_LDFLAGS := $(SL_LIB)





include ../quickbuild.make
