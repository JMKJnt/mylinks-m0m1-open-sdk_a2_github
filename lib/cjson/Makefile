include $(TOPDIR)/.lib_config

OBJS = cJSON.o
#OBJS += test_lynx.o

include $(TOPDIR)/rules.mak
all: $(LIBA)
clean:
	find -regex '.*\.[oasd]' -print | xargs rm -f
