
OBJS = at_command.o
OBJS += php.o
OBJS +=at_user.o


-include appconf.mak
-include $(TOPDIR)/.lib_config
-include $(TOPDIR)/rules.mak


CPPFLAGS += -I$(TOPDIR)/include -include lib_autoconf.h -DCONFIG_DATABUF_START=$(CONFIG_DATABUF_START) -DCONFIG_DMA_SIZE=$(CONFIG_DMA_SIZE) -DCONFIG_USB_CDC_ACM_DEV=$(CONFIG_USB_CDC_ACM_DEV)

LIBS = $(LIBPATH)/sys.a
LIBS += $(LIBPATH)/libatmain.a
LIBS += $(LIBPATH)/atcmd.a
LIBS += $(LIBPATH)/webserver.a
LIBS += $(LIBPATH)/sys.a 
LIBS += $(LIBPATH)/common.a
LIBS += $(LIBPATH)/lwip.a
LIBS += $(LIBPATH)/wla.a 
LIBS += $(LIBPATH)/freertos.a
LIBS += $(LIBPATH)/cJSON.a
LIBS += $(LIBPATH)/minifs.a
LIBS += $(LIBPATH)/cJSON.a
LIBS += $(LIBPATH)/mlink.a


all: all_app

app2: proj.ld $(OBJS:.o=.d) $(LIBS)
	$(ECHO) "\t [LD] $@"
	$(XCC) $(LDFLAGS) -nostdlib -Wl,-e,app_main -T./proj.ld  -o $@ $(OBJS) $(LIBS)

app2.img: app2 app2.map
	$(ECHO) "\t[IMG] $<"
	$(XOC) -R .bss -R .dma -O binary  $< $@

ifeq "$(CONFIG_WEB_AS_DATA)" "y"

www.img: $(TOPDIR)/lib/www/www_desc.o
	$(Q)$(XOC) -O binary $< $@

app_1.img: app2.img im_header.bin $(TOPDIR)/utility/chksum
	@cat im_header.bin app2.img > $@.cs
	@$(TOPDIR)/utility/chksum $@.cs $@
	@rm -f *.bin *.cs app2.img

app_2.img: www.img $(LIBPATH)/im_header.c $(TOPDIR)/include/image.h $(TOPDIR)/utility/chksum
	$(Q)$(XCC) -o $@.o -c -I$(TOPDIR)/include -I./ $(LIBPATH)/im_header.c -DIH_RUN=0 \
		 -DIH_SIZE=$(shell stat -c%s $<) -DIH_CHKSUM=0 -DIH_NEXT=0 -DIH_VER=$(__REV) -w
	$(Q)$(XOC) -j .data -O binary $@.o $@.bin
	@cat $@.bin $< $(TOPDIR)/lib/www/pages.img > $@.zcs
	@$(TOPDIR)/utility/chksum $@.zcs $@
	@rm -f $@.zcs $@.o $@.bin www.img

app.img: app_1.img app_2.img
	$(ECHO) "\t[IMG] $@"
	@cat app_1.img app_2.img > $@
	@rm app_1.img app_2.img
else
app.img: app2.img im_header.bin $(TOPDIR)/utility/chksum
	@cat im_header.bin app2.img > $@.cs
	@$(TOPDIR)/utility/chksum $@.cs $@
	@rm -f *.bin *.cs app2.img
endif

all_app: app.img
	mv app.img $(TOPDIR)/images/$(PROJ_NAME).img

clean:
	$(ECHO) "\t[CLEAN]"
	@find . \( -name '*.[osd]' -o -name '*.map' -o -name '*.bin' -o -name '*.dis' -o -name '*.ld' \) -type f -print| xargs rm -f
	@rm -f app2 im_header app.img.cs
