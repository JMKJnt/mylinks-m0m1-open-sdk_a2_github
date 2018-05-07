#
# Top level Makefile
#
IMAGE:=images
__REV=1944
TOPDIR=$(shell pwd)
IMAGEDIR:=$(TOPDIR)/$(IMAGE)
BOOTFILE:=$(IMAGEDIR)/iot_boot.img
IMAGEFILE:=$(IMAGEDIR)/main.img
MPFILE:=$(IMAGEDIR)/mp_test_uart.img
ALLFILE:=all-firmware.img



LIBPATH=$(TOPDIR)/lib
export TOPDIR LIBPATH __REV
PROJ_PATH:= $(shell find proj/ -maxdepth 1 -type d)
PROJ_DIR:= $(notdir $(PROJ_PATH))
PROJ_DIR_CLEAN:= $(PROJ_DIR:%=clean-%)

BUILT_H	= $(TOPDIR)/include/built_info.h
ifeq ($(BUILT_H),$(wildcard $(BUILT_H)))
	BUILT_C=$(shell sed -n 's/.*_COUNT\s\([0-9]*\)/\1/p' $(BUILT_H))
else
	BUILT_C=0
endif

-include toolchain.mak # only exist in SDK (based on sdk_toolchain.mak)
-include .lib_config
-include rules.mak
-include develop.mak

lib:
	$(Q)$(MAKE) -C lib all

libm:
	$(Q)$(MAKE) -C lib module

lib/%/build:
	$(Q)$(MAKE) -C lib $(@:lib/%/build=_dir_%)

$(BUILT_H):
	@echo "/*" > $@
	@echo "		built information" >> $@
	@echo "*/" >> $@
	@echo "#define SW_BUILD_TIME \"$(shell date +"%Y%m%d_%H%M")\""  >> $@
	@echo "#define SW_BUILD_COUNT $(shell expr $(BUILT_C) + 1)" >> $@

nomp_1m:
	@touch $(IMAGEDIR)/$(ALLFILE)
	tr  "\000" "\377" < /dev/zero | dd of=$(IMAGEDIR)/$(ALLFILE) ibs=1 count=1048576
	@dd if=$(BOOTFILE) of=$(IMAGEDIR)/$(ALLFILE) conv=notrunc
	@dd if=$(IMAGEFILE) of=$(IMAGEDIR)/$(ALLFILE) ibs=324K obs=128K count=1 seek=1 conv=notrunc
	@dd if=$(MPFILE) of=$(IMAGEDIR)/$(ALLFILE) ibs=128K obs=896K count=1 seek=1 conv=notrunc

	
$(TOPDIR)/utility/chksum: $(TOPDIR)/utility/chksum.c
	rm -rf $(TOPDIR)/utility/chksum
	gcc -o $@ $< -lssl -lcrypto

$(PROJ_DIR): $(BUILT_H) $(TOPDIR)/utility/chksum
	$(Q)$(MAKE) -C proj/$@ all
# append all modules to firmware image
#	$(Q)$(MAKE) -C proj/$@ app2.lds
#	$(Q)ln -sf proj/$@/app2.lds app2.lds
#	$(Q)$(MAKE) libm
#	$(Q)cat lib/*.m >> $(TOPDIR)/images/$@.img

$(PROJ_DIR_CLEAN):
	$(Q)$(MAKE) -C proj/$(@:clean-%=%) clean

clean-all: $(PROJ_DIR_CLEAN)

.PHONY: lib libm lib/%/build $(BUILT_H) $(PROJ_DIR) $(PROJ_DIR_CLEAN) clean-all
