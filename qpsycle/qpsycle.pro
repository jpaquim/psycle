# Use qmake to generate your Makefile from this file.
# N.B. dont run qmake --project! It will overwrite 
# this file and we don't want that.

TEMPLATE = app # This project builds an executable program.
TARGET = 

CONFIG += thread
!warn_on: CONFIG += warn_off
message( "Config is $${CONFIG}." )

BUILD_DIR = build
OBJECTS_DIR = $$BUILD_DIR # Where the .o files go.
MOC_DIR = $$BUILD_DIR # Where intermediate moc files go.
#DESTDIR = $$BUILD_DIR # Where the final executable goes.

DEPENDPATH += . \
	src/ \
	src/gui \
	src/gui/machineview \
	src/gui/patternview \
	psycore \
	src/gui/sequencer \
	src/gui/waveview \
	psycore/helpers \
	psycore/helpers/math
	
INCLUDEPATH += . \
	src/ \
	src/gui \
	src/gui/machineview \
	src/gui/patternview \
	src/gui/waveview \
	src/gui/sequencer \
	src/audiodrivers \
	src/mididrivers

# Input
HEADERS += \
	src/gui/mainwindow.h \
	src/gui/global.h \
	src/gui/configuration.h \
	src/gui/inputhandler.h \
	src/gui/patternbox.h \
	src/gui/audioconfigdlg.h \
	src/gui/machineview/machinegui.h \
	src/gui/machineview/machinetweakdlg.h \
	src/gui/machineview/machineview.h \
	src/gui/machineview/newmachinedlg.h \
	src/gui/machineview/wiregui.h \
	src/gui/machineview/generatorgui.h \
	src/gui/machineview/mastergui.h \
	src/gui/machineview/effectgui.h \
	src/gui/patternview/patternview.h \
	src/gui/patternview/linenumbercolumn.h \
	src/gui/patternview/trackheader.h \
	src/gui/patternview/patterngrid.h \
	src/gui/patternview/patterndraw.h \
	src/gui/sequencer/sequencerarea.h \
	src/gui/sequencer/sequencerdraw.h \
	src/gui/sequencer/sequenceritem.h \
	src/gui/sequencer/sequencerline.h \
	src/gui/sequencer/sequencerview.h \
	src/gui/sequencer/beatruler.h \
	src/gui/waveview/waveview.h \
	audiodrivers/audiodriver.h \
	audiodrivers/wavefileout.h \
	psycore/binread.h \
	psycore/constants.h \
	psycore/convert_internal_machines.h \
	psycore/cstdint.h \
	psycore/datacompression.h \
	psycore/dither.h \
	psycore/dsp.h \
	psycore/eventdriver.h \
	psycore/fileio.h \
	psycore/filter.h \
	psycore/helpers.h \
	psycore/instpreview.h \
	psycore/instrument.h \
	psycore/internal_machines.h \
	psycore/ladspa.h \
	psycore/ladspamachine.h \
	psycore/machine.h \
	psycore/mersennetwister.h \
	psycore/pattern.h \
	psycore/patterndata.h \
	psycore/patternevent.h \
	psycore/patternline.h \
	psycore/patternsequence.h \
	psycore/player.h \
	psycore/playertimeinfo.h \
	psycore/plugin.h \
	psycore/plugin_interface.h \
	psycore/pluginfinder.h \
	psycore/preset.h \
	psycore/psy2filter.h \
	psycore/psy3filter.h \
	psycore/psy4filter.h \
	psycore/psyfilter.h \
	psycore/riff.h \
	psycore/sampler.h \
	psycore/sigslot.h \
	psycore/singlepattern.h \
	psycore/song.h \
	psycore/timesignature.h \
	psycore/xminstrument.h \
	psycore/xmsampler.h \
	psycore/zipreader.h \
	psycore/zipwriter.h \
	psycore/zipwriterstream.h \
	psycore/file.h \
	psycore/helpers/scale.hpp \
	psycore/helpers/math/pi.hpp \
	psycore/helpers/xml.h

SOURCES += \
	src/gui/mainwindow.cpp \
	src/gui/global.cpp \
	src/gui/configuration.cpp \
	src/gui/inputhandler.cpp \
	src/gui/patternbox.cpp \
	src/gui/audioconfigdlg.cpp \
	qpsycle.cpp \
	src/gui/machineview/machinegui.cpp \
	src/gui/machineview/machinetweakdlg.cpp \
	src/gui/machineview/machineview.cpp \
	src/gui/machineview/newmachinedlg.cpp \
	src/gui/machineview/wiregui.cpp \
	src/gui/machineview/generatorgui.cpp \
	src/gui/machineview/mastergui.cpp \
	src/gui/machineview/effectgui.cpp \
	src/gui/patternview/patternview.cpp \
	src/gui/patternview/linenumbercolumn.cpp \
	src/gui/patternview/trackheader.cpp \
	src/gui/patternview/patterngrid.cpp \
	src/gui/patternview/patterndraw.cpp \
	src/gui/sequencer/sequencerarea.cpp \
	src/gui/sequencer/sequencerdraw.cpp \
	src/gui/sequencer/sequenceritem.cpp \
	src/gui/sequencer/sequencerline.cpp \
	src/gui/sequencer/sequencerview.cpp \
	src/gui/sequencer/beatruler.cpp \
	src/gui/waveview/waveview.cpp \
	audiodrivers/audiodriver.cpp \
	audiodrivers/wavefileout.cpp \
	psycore/binread.cpp \
	psycore/convert_internal_machines.cpp \
	psycore/datacompression.cpp \
	psycore/dither.cpp \
	psycore/dsp.cpp \
	psycore/eventdriver.cpp \
	psycore/fileio.cpp \
	psycore/filter.cpp \
	psycore/helpers.cpp \
	psycore/instpreview.cpp \
	psycore/instrument.cpp \
	psycore/internal_machines.cpp \
	psycore/ladspamachine.cpp \
	psycore/machine.cpp \
	psycore/mersennetwister.cpp \
	psycore/patterndata.cpp \
	psycore/patternevent.cpp \
	psycore/patternline.cpp \
	psycore/patternsequence.cpp \
	psycore/player.cpp \
	psycore/playertimeinfo.cpp \
	psycore/plugin.cpp \
	psycore/pluginfinder.cpp \
	psycore/preset.cpp \
	psycore/psy2filter.cpp \
	psycore/psy3filter.cpp \
	psycore/psy4filter.cpp \
	psycore/psyfilter.cpp \
	psycore/riff.cpp \
	psycore/sampler.cpp \
	psycore/singlepattern.cpp \
	psycore/song.cpp \
	psycore/timesignature.cpp \
	psycore/xminstrument.cpp \
	psycore/xmsampler.cpp \
	psycore/zipreader.cpp \
	psycore/zipwriter.cpp \
	psycore/zipwriterstream.cpp \
	psycore/file.cpp \
	psycore/helpers/xml.cpp

RESOURCES += qpsycle.qrc

QT += xml

unix {
	include(qpsycle-unix.pro)
}

win32 {
	include(qpsycle-win32.pro)
}

message( "LIBS are: [$${LIBS}]" )
