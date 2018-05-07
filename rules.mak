#
# rules.mak
#

ARCH = ba22
CPU = -march=ba2 -mbe -mabi=3 -mtj-rodata -mno-hard-div -mno-hard-mul -ffixed-r31 -ffixed-r30 -G0
CROSS_PREFIX=$(TOOLCHAIN_PATH)ba-elf-

#CPPFLAGS
CPPFLAGS += -I. -I./include
CPPFLAGS += -I$(TOPDIR)/include/freertos -I$(TOPDIR)/lib/lwip/include -I$(TOPDIR)/lib/lwip -I$(TOPDIR)/lib/lwip/include/ipv4
CPPFLAGS += -I$(TOPDIR)/lib/mqtt/include -I$(TOPDIR)/lib/cjson/include
CPPFLAGS += -D__REV=$(__REV)
#CFLAGS
CFLAGS += -ffunction-sections -fdata-sections
CFLAGS += -fno-strict-aliasing -fno-common $(CPU) -Os -g -fregmove
CFLAGS += -fno-builtin
CFLAGS += -Werror
CFLAGS += $(CFLAGS_GCC)
ifeq (1,$(IPO))
CFLAGS += -flto -fwhole-program
endif
CFLAGS_src/start.o = -Wa,-gstabs
#LDFLAGS
LDFLAGS += -nostartfiles -Wl,-static
LDFLAGS += -Wl,-gc-sections

ifndef	V
Q=@
ECHO=@echo
MAKE=make -s
else
Q=
ECHO=@\#
MAKE=make
endif
XCC = $(CROSS_PREFIX)gcc
XLD = $(CROSS_PREFIX)ld
XOC = $(CROSS_PREFIX)objcopy
XNM = $(CROSS_PREFIX)nm
XOD = $(CROSS_PREFIX)objdump
XAR = $(CROSS_PREFIX)ar
XSZ = $(CROSS_PREFIX)size
D2U = dos2unix

# RULES
%.o: %.o.gz
	$(ECHO) "\t[GZIP] $<"
	$(Q)gzip -d -c $< > $*.o

%.a: %.a.gz
	$(ECHO) "\t[GZIP] $<"
	$(Q)gzip -d -c $< > $*.a

%.d: %.o.gz
	$(ECHO) "\t[GZIP] $<"
	$(Q)gzip -d -c $< > $*.o

%.d: %.c
	$(ECHO) "\t [CC] $<"
	$(Q)$(XCC) -c -o $(@:.d=.o) $(CFLAGS) $(CFLAGS_$(@:.d=.o)) $(CPPFLAGS) $(CPPFLAGS_$(@:.d=.o)) -Wp,-MD,$(@:.d=.tmp) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.d=.tmp) > $@ ; rm $(@:.d=.tmp)

%.d: %.S
	$(ECHO) "\t[ASM] $<"
	$(Q)$(XCC) -c -o $(@:.d=.o) $(CFLAGS) $(CFLAGS_$(@:.d=.o)) $(CPPFLAGS) $(CPPFLAGS_$(@:.d=.o)) -Wp,-MD,$(@:.d=.tmp) $<
	@sed -e '/^ *\\/d' -e "s#.*: #$@: #" $(@:.d=.tmp) > $@ ; rm $(@:.d=.tmp)

%.img: %
	$(ECHO) "\t[IMG] $<"
	$(Q)$(XOC) -O binary -R .dma $< $@

%.i: %.c
	$(XCC) -E -o $*.i $(CPPFLAGS) $(CPPFLAGS_$(@:.i=.o)) $<

%.map: %
	$(ECHO) "\t[MAP] $<"
	$(Q)$(XNM) $(@:.map=) | grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)' | sort > $@

%.dis: %
	$(XOD) -S --show-raw-insn $< > $@

%.dis: %.o
	$(XOD) -S --show-raw-insn $< > $@

%.sz: %
	$(XNM) -r --size-sort $< > $@

im_header.bin: $(LIBPATH)/im_header.c $(TOPDIR)/include/app_hdr.ld
	@$(XCC) -o im_header.o -c -I$(TOPDIR)/include -I./ $(LIBPATH)/im_header.c -DIH_RUN=$(shell sed -n 's/\([0-9a-f]*\) [A-Z] app_main/0x\1/p' app2.map) -DIH_SIZE=$(shell stat -c%s app2.img) -DIH_CHKSUM=0 -DIH_NEXT=8 -DIH_VER=$(__REV) -w -include lib_autoconf.h
	@$(XCC) -nostartfiles -Wl,-static -nostdlib -T$(TOPDIR)/include/app_hdr.ld -o im_header im_header.o
	@$(XOC) -j .data -O binary im_header $@

im_header_zcfg.bin: $(LIBPATH)/im_header.c $(TOPDIR)/include/app_hdr.ld
	@$(XCC) -o im_header.o -c -I$(TOPDIR)/include -I./ $(LIBPATH)/im_header.c -DIH_RUN=$(shell sed -n 's/\([0-9a-f]*\) [A-Z] app_main/0x\1/p' app_zcfg.map) -DIH_SIZE=$(shell stat -c%s app_zcfg.img) -DIH_CHKSUM=0 -DIH_NEXT=8 -DIH_VER=$(__REV) -w -include lib_autoconf.h
	@$(XCC) -nostartfiles -Wl,-static -nostdlib -T$(TOPDIR)/include/app_hdr.ld -o im_header im_header.o
	@$(XOC) -j .data -O binary im_header $@

rp_str1 = "s/CONFIG_DATABUF_START/"$(CONFIG_DATABUF_START)"/g;s/CONFIG_DMA_SIZE/"$(CONFIG_DMA_SIZE)"/g"

proj.ld: $(TOPDIR)/include/app.ld
	sed -e $(rp_str1) $< > $@
ifdef CONFIG_FPGA
	echo "INCLUDE \""$(TOPDIR)"/include/rlib_sym.lds\"" >> $@
else
ifdef CONFIG_RLIB_COMPATIBLE
	echo "INCLUDE \""$(TOPDIR)"/include/rlib_sym_v2v3_common.lds\"" >> $@
else
	echo "INCLUDE \""$(TOPDIR)"/include/rlib_sym_v"$(CONFIG_ROM_VER)".lds\"" >> $@
endif
endif

app2.lds: app2
	$(ECHO) "\t[LDS] $(PROJ_PATH)"
	$(ECHO) "\t[LDS] $@"
	$(XNM) -gn app2 | sed '/^0/!d' | sed -e 's/\([0-9a-f]*\) [A-Z] \(.*\)/PROVIDE( \2 = 0x\1 );/' > $@

LIBA = $(notdir $(shell pwd)).a
LIBM = $(notdir $(shell pwd)).m

MOD_CFLAGS = -fno-zero-initialized-in-bss -nostdlib -ffixed-r29 -T $(TOPDIR)/include/module_offset.lds -T $(TOPDIR)/app2.lds -T $(TOPDIR)/include/rlib_sym_v$(CONFIG_ROM_VER).lds $(CFLAGS) $(CPPFLAGS) $(CFLAGS_$(@:.d=.o))
MOD_LDFLAGS = --strip-debug --emit-relocs -nostdlib -T $(TOPDIR)/include/module_offset.lds -T $(TOPDIR)/app2.lds -T $(TOPDIR)/include/rlib_sym_v$(CONFIG_ROM_VER).lds

$(LIBA): $(OBJS:.o=.d)
	$(ECHO) "\t[AR] $@"
	$(Q)$(XAR) rsc $@ $(OBJS)
	mv $@ ../

$(LIBM): $(TOPDIR)/app2.lds
	$(ECHO) "\t[MOD] $@"
	$(Q)$(XCC) $(MOD_CFLAGS) -o a.out $(OBJS:.o=.c)
	$(Q)$(XCC) -S $(MOD_CFLAGS) $(OBJS:.o=.c)
	$(Q)$(XNM) a.out | grep "[0-9a-f]\{8\} A" | awk '{print $$3}' | while read s; do sed -i -e "s/b.jal  \t\($$s\)/bw.ori\tr29,r0,\1\n\tb.jalr\tr29/" -e "s/b.j    \t\($$s\)/bw.ori\tr29,r0,\1\n\tb.jr\tr29/" $(OBJS:.o=.s); done
	$(Q)rm a.out
	$(Q)$(XCC) -c $(MOD_CFLAGS) $(OBJS:.o=.s)
	$(Q)$(XLD) $(MOD_LDFLAGS) -o $@ $(OBJS)
	$(Q)$(XOC) --change-address 0x54 --remove-section .comment $@ #decrease file size
	$(Q)stat -c %s $@ | xargs printf 0x%08x | xxd -r -p | dd of=$@ bs=1 count=4 seek=20 conv=notrunc 2> /dev/null #store file size into version
	$(Q)dd of=$@ bs=1 count=32 seek=52 conv=notrunc < /dev/zero 2> /dev/null #fill zero into program header table
	$(Q)echo -n $@ | dd of=$@ bs=1 count=32 seek=52 conv=notrunc 2> /dev/null #store file name into program header table
	$(Q)mv $@ ../

PROJ_NAME = `pwd | sed 's,.*/,,'`

ifneq ($(MAKECMDGOALS),clean)
ifneq "" "$(OBJS:.o=.d)"
-include $(OBJS:.o=.d)
endif
endif
