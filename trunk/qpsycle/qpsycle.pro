# Use qmake to generate your Makefile from this file.
# N.B. dont run qpsycle --project! It will overwrite 
# this file and we don't want that.

TEMPLATE = app
TARGET = 
CONFIG += warn_off debug
OBJECTS_DIR = build # Where the .o files go.
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
           patternview/trackheader.h \
           patternview/patterngrid.h \
           patternview/patterndraw.h \
           audiodrivers/audiodriver.h \
           psycore/binread.h \
           psycore/configuration.h \
           psycore/constants.h \
           psycore/convert_internal_machines.h \
           psycore/cstdint.h \
           psycore/datacompression.h \
           psycore/dither.h \
           audiodrivers/dsound.h \
           psycore/dsp.h \
           psycore/eventdriver.h \
           psycore/fileio.h \
           psycore/filter.h \
           psycore/global.h \
           audiodrivers/gstreamerout.h \
           psycore/helpers.h \
           psycore/inputhandler.h \
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
           psycore/songstructs.h \
           psycore/timesignature.h \
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
           psycore/helpers/math/pi.hpp \
           psycore/helpers/xml.h
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
           patternview/trackheader.cpp \
           patternview/patterngrid.cpp \
           patternview/patterndraw.cpp \
           audiodrivers/audiodriver.cpp \
           psycore/binread.cpp \
           psycore/convert_internal_machines.cpp \
           psycore/configuration.cpp \
           psycore/datacompression.cpp \
           psycore/dither.cpp \
           psycore/dsp.cpp \
           psycore/eventdriver.cpp \
           psycore/fileio.cpp \
           psycore/filter.cpp \
           psycore/global.cpp \
           audiodrivers/gstreamerout.cpp \
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
           waveview/waveview.cpp \
           psycore/helpers/xml.cpp

RESOURCES += qpsycle.qrc

QT += xml

unix {
    message("System is: unix.")
    CONFIG += link_pkgconfig
    system( pkg-config --exists alsa ) {
        message( "pkg-config thinks alsa libs are available..." )
        DEFINES += QPSYCLE__ALSA_AVAILABLE # This is used in the source to determine when
                                           # to include alsa-specific things.
        PKGCONFIG += alsa 
        INCLUDEPATH += /usr/include/alsa 
        HEADERS += audiodrivers/alsaout.h \
                   mididrivers/alsaseqin.h 
        SOURCES += audiodrivers/alsaout.cpp \
                   mididrivers/alsaseqin.cpp 
    }
    system( pkg-config --exists jack ) {
        message( "pkg-config thinks jack libs are available..." )
        DEFINES += QPSYCLE__JACK_AVAILABLE # This is used in the source to determine when
                                           # to include jack-specific things.
        PKGCONFIG += jack 
        HEADERS += audiodrivers/jackout.h 
        SOURCES += audiodrivers/jackout.cpp 
    }
    system( pkg-config --exists esound ) {
        message( "esd-config thinks esd libs are available..." )
        DEFINES += QPSYCLE__ESD_AVAILABLE  
        LIBS += $$system( esd-config --libs )
        HEADERS += audiodrivers/esoundout.h 
        SOURCES += audiodrivers/esoundout.cpp 
    }
    LIBS += -lboost_signals
    # FIXME: not sure how to test for netaudio...
    LIBS += -laudio
    HEADERS += audiodrivers/netaudioout.h \
           audiodrivers/wavefileout.h 
    SOURCES += audiodrivers/netaudioout.cpp \
           audiodrivers/wavefileout.cpp 
}
win32 {
    INCLUDEPATH += C:/Qt/4.2.2/src/3rdparty/zlib

    INCLUDEPATH += ../external-packages/boost-1.33.1/include
    # for mingw
    LIBPATH += ../external-packages/boost-1.33.1/lib-mswindows-mingw-cxxabi-1002
    # for msvc
    #LIBPATH += ../external-packages/boost-1.33.1/lib-mswindows-msvc-8.0-cxxabi-1400

    HEADERS += audiodrivers/microsoft_direct_sound_out.h \
           audiodrivers/msdirectsound.h \
           audiodrivers/mswaveout.h 
    SOURCES += audiodrivers/microsoft_direct_sound_out.cpp \
           audiodrivers/msdirectsound.cpp \
           audiodrivers/mswaveout.cpp 
}
