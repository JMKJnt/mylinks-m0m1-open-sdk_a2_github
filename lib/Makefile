-include $(TOPDIR)/.lib_config
export TOPDIR 

CPPFLAGS = -I$(TOPDIR)/include -include lib_autoconf.h
CPPFLAGS += -I$(TOPDIR)/lib/lwip/
CPPFLAGS += -I$(TOPDIR)/lib/lwip/include
CPPFLAGS += -I$(TOPDIR)/lib/lwip/include/ipv4
export CPPFLAGS


-include libconf.mak

LIBDIRS = $(libdirs-y)
LIBS = $(LIBDIRS:%=%.a) 

$(libdirs-y:%=_dir_%) :
	@if [ -e $(@:_dir_%=%)/Makefile ]; then echo "\n      [$(@:_dir_%=%)] .."; rm -f $(@:_dir_%=%).a; \
	$(MAKE) -C $(@:_dir_%=%) clean ; $(MAKE) -C $(@:_dir_%=%) CFLAGS="$(CFLAGS)" all; fi

$(libdirs-m:%=_dir_%) :
	@if [ -e $(@:_dir_%=%)/Makefile ]; then echo "\n      [$(@:_dir_%=%)] .."; rm -f $(@:_dir_%=%).m; \
	$(MAKE) -C $(@:_dir_%=%) clean ; $(MAKE) -C $(@:_dir_%=%) CFLAGS="$(CFLAGS)" all; fi

all: $(libdirs-y:%=_dir_%)

module: $(libdirs-m:%=_dir_%)

-include $(TOPDIR)/rules.mak

.PHONY: all module
