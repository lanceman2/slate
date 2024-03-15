
SL_LIB := -L../lib -lslate -Wl,-rpath=\$$ORIGIN/../lib

020_spew_SOURCES := 020_spew.c
020_spew_LDFLAGS := $(SL_LIB)
020_spew_CPPFLAGS := -I../include

030_app_SOURCES := 030_app.c
030_app_LDFLAGS := $(SL_LIB)


include ../quickbuild.make
