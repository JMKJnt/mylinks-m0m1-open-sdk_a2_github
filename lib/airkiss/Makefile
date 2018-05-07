include $(TOPDIR)/.lib_config



include $(TOPDIR)/rules.mak


all: 
	gzip -d -c libairkiss.a.gz > ../libairkiss.a
	gzip -d -c libairkiss_log.a.gz > ../libairkiss_log.a
	gzip -d -c libairkiss_aes.a.gz > ../libairkiss_aes.a
	gzip -d -c libairkiss_aes_log.a.gz > ../libairkiss_aes_log.a

clean:
	find -regex '.*\.[oasd]' -print | xargs rm -f
