HELPERDIRS := ../container/src ../dsp/src
PLUGDIRS :=\
			alk_muter/src\
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
			druttis_pluckedstring/src\			
			dw_eq/src\
			dw_granulizer/src\
			dw_iopan/src\
			dw_tremolo/src\
			flanger/src\
			filter_2_poles/src\
			gainer/src\
			gverb/src\
			graue_softsat/src\
			haas/src\
			jm_drums/src\
			jme_blitz12/src\
			jme_blitzn/src\
			jme_gamefx13/src\
			jme_gamefxn/src\
			legasynth/src\
			m3/src\
			moreamp_eq/src\
			negative/src\
			ninereeds_7900/src\
			phaser/src\
			pooplog_filter/src\
			pooplog-synth/src\
			pooplog-synth-light/src\
			pooplog-synth-ultralight/src\
			schorus/src\
			stk-plugged/src\
			stk-reverbs/src\
			stk-shakers/src\
			ring_modulator/src\
			yezar_freeverb/src\
			zephod_super_fm/src
SUBDIRS := $(HELPERDIRS) $(PLUGDIRS)
OUT_DIR := ./build

MKDIR_P = mkdir -p

.PHONY: directories

all: directories $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@

directories: ${OUT_DIR}

${OUT_DIR}:
	${MKDIR_P} ${OUT_DIR}

.PHONY: all $(SUBDIRS)

clean:
	rm -f $(addsuffix /*.o,$(PLUGDIRS))
	rm -f $(addsuffix /*.so,$(PLUGDIRS))
	rm -f build/*.so
