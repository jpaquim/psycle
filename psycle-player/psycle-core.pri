HEADERS += \
	src/psycore/binread.h \
	src/psycore/constants.h \
	src/psycore/convert_internal_machines.h \
	src/psycore/cstdint.h \
	src/psycore/datacompression.h \
	src/psycore/dither.h \
	src/psycore/dsp.h \
	src/psycore/eventdriver.h \
	src/psycore/fileio.h \
	src/psycore/filter.h \
	src/psycore/helpers.h \
	src/psycore/instpreview.h \
	src/psycore/instrument.h \
	src/psycore/internal_machines.h \
	src/psycore/ladspa.h \
	src/psycore/ladspamachine.h \
	src/psycore/machine.h \
	src/psycore/mersennetwister.h \
	src/psycore/pattern.h \
	src/psycore/patterndata.h \
	src/psycore/patternevent.h \
	src/psycore/patternline.h \
	src/psycore/patternsequence.h \
	src/psycore/player.h \
	src/psycore/playertimeinfo.h \
	src/psycore/plugin.h \
	src/psycore/plugin_interface.h \
	src/psycore/pluginfinder.h \
	src/psycore/preset.h \
	src/psycore/psy2filter.h \
	src/psycore/psy3filter.h \
	src/psycore/psy4filter.h \
	src/psycore/psyfilter.h \
	src/psycore/riff.h \
	src/psycore/sampler.h \
	src/psycore/sigslot.h \
	src/psycore/singlepattern.h \
	src/psycore/song.h \
	src/psycore/songstructs.h \
	src/psycore/timesignature.h \
	src/psycore/xminstrument.h \
	src/psycore/xmsampler.h \
	src/psycore/zipreader.h \
	src/psycore/zipwriter.h \
	src/psycore/zipwriterstream.h \
	src/psycore/file.h \
	src/psycore/helpers/scale.hpp \
	src/psycore/helpers/math/pi.hpp \
	src/psycore/helpers/xml.h
	
SOURCES += \
	src/psycore/binread.cpp \
	src/psycore/convert_internal_machines.cpp \
	src/psycore/datacompression.cpp \
	src/psycore/dither.cpp \
	src/psycore/dsp.cpp \
	src/psycore/eventdriver.cpp \
	src/psycore/fileio.cpp \
	src/psycore/filter.cpp \
	src/psycore/helpers.cpp \
	src/psycore/instpreview.cpp \
	src/psycore/instrument.cpp \
	src/psycore/internal_machines.cpp \
	src/psycore/ladspamachine.cpp \
	src/psycore/machine.cpp \
	src/psycore/mersennetwister.cpp \
	src/psycore/patterndata.cpp \
	src/psycore/patternevent.cpp \
	src/psycore/patternline.cpp \
	src/psycore/patternsequence.cpp \
	src/psycore/player.cpp \
	src/psycore/playertimeinfo.cpp \
	src/psycore/plugin.cpp \
	src/psycore/pluginfinder.cpp \
	src/psycore/preset.cpp \
	src/psycore/psy2filter.cpp \
	src/psycore/psy3filter.cpp \
	src/psycore/psy4filter.cpp \
	src/psycore/psyfilter.cpp \
	src/psycore/riff.cpp \
	src/psycore/sampler.cpp \
	src/psycore/singlepattern.cpp \
	src/psycore/song.cpp \
	src/psycore/timesignature.cpp \
	src/psycore/xminstrument.cpp \
	src/psycore/xmsampler.cpp \
	src/psycore/zipreader.cpp \
	src/psycore/zipwriter.cpp \
	src/psycore/zipwriterstream.cpp \
	src/psycore/file.cpp \
	src/psycore/helpers/xml.cpp

include(boost.pri)
include(qt-xml.pri)
