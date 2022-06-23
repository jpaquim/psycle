SUBDIRS := ../script/src ../thread/src ../container/src ../file/src ../ui/src ../luaui/src ../dsp/src ../audio/src ../driver/alsa ../driver/jack ../host/src 

all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: all $(SUBDIRS)
clean:
	rm -f ../container/src/*.o ../dsp/src/*.o ../audio/src/*.o ../driver/alsa/*.o ../file/src/*.o ../src/*.o ../ui/src/*.o ../ui/src/imps/x11/*.o src/*.o ../container/src/*.a ../dsp/src/*.a ../audio/src/*.a ../driver/alsa/*.so ../driver/jack/*.so ../file/src/*.a ../src/*.a ../ui/src/*.a ../luaui/src/*.a ./src/*.o ./src/psycle ../script/src/*.o ../thread/src/*.o 
