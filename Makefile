# This file is for building and installing slate with quickbuild.
#

SUBDIRS :=\
 include\
 lib\
 bin


ifneq ($(wildcard quickbuild.make),quickbuild.make)
$(error "First run './bootstrap'")
endif
ifneq ($(wildcard config.make),config.make)
$(error "Now run './configure'")
endif



ifeq ($(strip $(subst cleaner, clean, $(MAKECMDGOALS))),clean)
SUBDIRS +=\
 tests
endif


test:
	cd tests && $(MAKE) test


include quickbuild.make
