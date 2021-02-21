SUBDIRS := ../container/src ../file/src ../dsp/src ../audio/src ../player/src ../driver/alsa

all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)
