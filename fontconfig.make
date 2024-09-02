
# Get the fontconfig specific compiler options if we can.
FONTCONFIG_LDFLAGS := $(shell pkg-config --libs fontconfig)
FONTCONFIG_CFLAGS := $(shell pkg-config --cflags fontconfig)

ifeq ($(FONTCONFIG_LDFLAGS),)
$(error software package fontconfig was not found)
else
# Spew what fontconfig compiler options we have found
$(info FONTCONFIG_CFLAGS="$(FONTCONFIG_CFLAGS)" FONTCONFIG_LDFLAGS="$(FONTCONFIG_LDFLAGS)")
endif
