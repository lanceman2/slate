
SL_LIB := -L../lib -lslate -Wl,-rpath=\$$ORIGIN/../lib

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







include ../quickbuild.make
