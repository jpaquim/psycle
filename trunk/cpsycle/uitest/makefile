SUBDIRS := ../container/src ../file/src ../audio/src ../ui/src ../uitest/src

all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)
clean:
	rm -f ../container/src/*.o ../dsp/src/*.o ../file/src/*.o ../src/*.o ../ui/src/*.o ../ui/src/imps/x11/*.o src/*.o src/uitest ../container/src/*.a ../dsp/src/*.a ../file/src/*.a
