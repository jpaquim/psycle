# Use qmake to generate your Makefile from this file.
# N.B. dont run qpsycle --project! It will overwrite 
# this file and we don't want that.

TEMPLATE = app
TARGET = 
LIBS = 
DEPENDPATH += . \
              machineview \
              patternview \
              psycore \
              sequencer \
              waveview \
              psycore/helpers \
              psycore/helpers/math
INCLUDEPATH += . \
               machineview \
               psycore \
               patternview \
               waveview \
               sequencer \
               psycore/helpers \
               psycore/helpers/math \

# Input
HEADERS += mainwindow.h \
           patternbox.h \
           machineview/machinegui.h \
           machineview/machinetweakdlg.h \
           machineview/machineview.h \
           machineview/newmachinedlg.h \
           machineview/wiregui.h \
           machineview/mastergui.h \
           machineview/effectgui.h \
           patternview/patternview.h \
           patternview/linenumbercolumn.h \
           patternview/header.h \
           patternview/patterngrid.h \
           patternview/patterndraw.h \
           psycore/abstractmachinefactory.h \
           psycore/audiodriver.h \
           psycore/binread.h \
           psycore/configuration.h \
           psycore/constants.h \
           psycore/convert_internal_machines.h \
           psycore/cstdint.h \
           psycore/datacompression.h \
           psycore/dither.h \
           psycore/dsound.h \
           psycore/dsp.h \
           psycore/eventdriver.h \
           psycore/fileio.h \
           psycore/filter.h \
           psycore/global.h \
           psycore/gstreamer_conditional_build.h \
           psycore/gstreamerout.h \
           psycore/helpers.h \
           psycore/inputhandler.h \
           psycore/install_paths.h \
           psycore/instpreview.h \
           psycore/instrument.h \
           psycore/internal_machines.h \
           psycore/ladspa.h \
           psycore/ladspa_conditional_build.h \
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
           psycore/songstructs.h \
           psycore/steinberg_asio_conditional_build.h \
           psycore/timesignature.h \
           psycore/wavefileout.h \
           psycore/xminstrument.h \
           psycore/xmsampler.h \
           psycore/zipreader.h \
           psycore/zipwriter.h \
           psycore/zipwriterstream.h \
           psycore/file.h \
           sequencer/sequencerarea.h \
           sequencer/sequencerdraw.h \
           sequencer/sequenceritem.h \
           sequencer/sequencerline.h \
           sequencer/sequencerview.h \
           sequencer/beatruler.h \
           waveview/waveview.h \
           psycore/helpers/scale.hpp \
           psycore/helpers/math/pi.hpp
SOURCES += mainwindow.cpp \
           patternbox.cpp \
           qpsycle.cpp \
           machineview/machinegui.cpp \
           machineview/machinetweakdlg.cpp \
           machineview/machineview.cpp \
           machineview/newmachinedlg.cpp \
           machineview/wiregui.cpp \
           machineview/mastergui.cpp \
           machineview/effectgui.cpp \
           patternview/patternview.cpp \
           patternview/linenumbercolumn.cpp \
           patternview/header.cpp \
           patternview/patterngrid.cpp \
           patternview/patterndraw.cpp \
           psycore/abstractmachinefactory.cpp \
           psycore/audiodriver.cpp \
           psycore/binread.cpp \
           psycore/configuration.cpp \
           psycore/datacompression.cpp \
           psycore/dither.cpp \
           psycore/dsp.cpp \
           psycore/eventdriver.cpp \
           psycore/fileio.cpp \
           psycore/filter.cpp \
           psycore/global.cpp \
           psycore/gstreamerout.cpp \
           psycore/helpers.cpp \
           psycore/inputhandler.cpp \
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
           psycore/wavefileout.cpp \
           psycore/xminstrument.cpp \
           psycore/xmsampler.cpp \
           psycore/zipreader.cpp \
           psycore/zipwriter.cpp \
           psycore/zipwriterstream.cpp \
           psycore/file.cpp \
           sequencer/sequencerarea.cpp \
           sequencer/sequencerdraw.cpp \
           sequencer/sequenceritem.cpp \
           sequencer/sequencerline.cpp \
           sequencer/sequencerview.cpp \
           sequencer/beatruler.cpp \
           waveview/waveview.cpp

RESOURCES += qpsycle.qrc

QT += xml

unix {
    LIBS += -lasound -lm -ldl -lpthread 
    INCLUDEPATH += /usr/include/alsa 
    HEADERS += psycore/alsa_conditional_build.h \
           psycore/alsaout.h \
           psycore/alsaseqin.h \
           psycore/netaudio_conditional_build.h \
           psycore/netaudioout.h 
    SOURCES += psycore/alsaout.cpp \
           psycore/alsaseqin.cpp \
           psycore/netaudioout.cpp 
}
win32 {
    INCLUDEPATH += C:/Qt/4.2.2/src/3rdparty/zlib
    HEADERS += psycore/microsoft_direct_sound_conditional_build.h \
           psycore/microsoft_direct_sound_out.h \
           psycore/msdirectsound.h \
           psycore/mswaveout.h 
    SOURCES += psycore/microsoft_direct_sound_out.cpp \
           psycore/msdirectsound.cpp \
           psycore/mswaveout.cpp 
}
