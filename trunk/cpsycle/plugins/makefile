SUBDIRS :=  alk_muter/src\
			arguru-compressor/src\
			arguru-distortion/src\
			arguru-goaslicer/src\
			arguru-reverb/src\
			arguru-synth-2f/src\
			arguru-xfilter/src\
			bexphase/src\
			compressor/src\
			crasher/src\
			delay/src\
			distortion/src\
			druttis_eq3/src\
			druttis_feedme/src\
			druttis_koruz/src\
			druttis_phantom/src\
			druttis_slicit/src\
			jme_blitz12/src\
			jme_blitzn/src\
			jme_gamefx13/src\
			jme_gamefxn/src\

all: $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: force
force :;

.PHONY: all $(SUBDIRS)

clean:
	rm -f $(addsuffix /*.o,$(SUBDIRS))
	rm -f $(addsuffix /*.so,$(SUBDIRS))
	rm -f build/*.so
