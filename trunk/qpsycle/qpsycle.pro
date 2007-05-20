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
           global.h \
           inputhandler.h \
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
           sequencer/sequencerarea.h \
           sequencer/sequencerdraw.h \
           sequencer/sequenceritem.h \
           sequencer/sequencerline.h \
           sequencer/sequencerview.h \
           sequencer/beatruler.h \
           waveview/waveview.h \
           audiodrivers/audiodriver.h \
           audiodrivers/wavefileout.h \
           audiodrivers/dsound.h \
           audiodrivers/gstreamerout.h \
           psycore/binread.h \
           psycore/configuration.h \
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
           psycore/songstructs.h \
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
SOURCES += mainwindow.cpp \
           global.cpp \
           inputhandler.cpp \
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
           sequencer/sequencerarea.cpp \
           sequencer/sequencerdraw.cpp \
           sequencer/sequenceritem.cpp \
           sequencer/sequencerline.cpp \
           sequencer/sequencerview.cpp \
           sequencer/beatruler.cpp \
           waveview/waveview.cpp \
           audiodrivers/audiodriver.cpp \
           audiodrivers/wavefileout.cpp \
           audiodrivers/gstreamerout.cpp \
           psycore/binread.cpp \
           psycore/convert_internal_machines.cpp \
           psycore/configuration.cpp \
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
    message("System is: unix.")
    CONFIG += link_pkgconfig # adds support for pkg-config via the PKG_CONFIG var

    LIBS += -lboost_signals

    system( pkg-config --exists alsa ) {
        message( "pkg-config thinks alsa libs are available..." )
        PKGCONFIG += alsa 
        DEFINES += PSYCLE__ALSA_AVAILABLE # This is used in the source to determine when to include alsa-specific things.
        HEADERS += audiodrivers/alsaout.h \
                   mididrivers/alsaseqin.h 
        SOURCES += audiodrivers/alsaout.cpp \
                   mididrivers/alsaseqin.cpp 
    }

    system( pkg-config --exists jack ) {
        message( "pkg-config thinks jack libs are available..." )
        PKGCONFIG += jack 
        DEFINES += PSYCLE__JACK_AVAILABLE # This is used in the source to determine when to include jack-specific things.
        HEADERS += audiodrivers/jackout.h
        SOURCES += audiodrivers/jackout.cpp 
    }

    system( pkg-config --exists esound ) {
        message( "pkg-config thinks esound libs are available..." )
        PKGCONFIG += esound
        DEFINES += PSYCLE__ESOUND_AVAILABLE # This is used in the source to determine when to include esound-specific things.
        HEADERS += audiodrivers/esoundout.h
        SOURCES += audiodrivers/esoundout.cpp 
    }

    false { # gstreamer output is unfinished
        system( pkg-config --exists gstreamer ) {
            message( "pkg-config thinks gstreamer libs are available..." )
            PKGCONFIG += gstreamer
            DEFINES += PSYCLE__GSTREAMER_AVAILABLE # This is used in the source to determine when to include gstreamer-specific things.
            HEADERS += audiodrivers/gstreamerout.h
            SOURCES += audiodrivers/gstreamerout.cpp 
        }
    }

    false { # note: the net audio output driver is probably not (well) polished/tested anyway. esound is a good alternative.
        # FIXME: not sure how to test for netaudio...
        exists(/usr/include/audio/audiolib.h) {
            LIBS += -laudio
            DEFINES += PSYCLE__NET_AUDIO_AVAILABLE # This is used in the source to determine when to include net-audio-specific things.
            HEADERS += audiodrivers/netaudioout.h
            SOURCES += audiodrivers/netaudioout.cpp
        }
    }
}
win32 {
    message("System is: win32.")
    win32-g++ {
        message("Compiler is: g++.")
    } else:win32-msvc {
        message("Compiler is: msvc.")
    } else {
        warning("Untested compiler.")
    }
       
    PWD = $$system(echo %CD%)
    EXTERNAL_PKG_DIR = $$PWD/../external-packages
    
    BOOST_DIR = $$EXTERNAL_PKG_DIR/boost-1.33.1
    !exists($$BOOST_DIR) {
        warning("The local boost dir does not exist: $${BOOST_DIR}. Make sure you have boost libs installed.")
    } else {
        !exists($$BOOST_DIR)/include {
            warning("The boost headers are not unpacked. See the dir $${BOOST_DIR}.")
        } else {
            INCLUDEPATH += $$BOOST_DIR/include
            win32-g++ {
                !exists($$BOOST_DIR/lib-mswindows-mingw-cxxabi-1002) {
                    warning("The boost libraries are not unpacked. See the dir $${BOOST_DIR}.")
                    # remove our local include path
                    INCLUDEPATH -= $$BOOST_DIR/include
                } else {
                    LIBPATH += $$BOOST_DIR/lib-mswindows-mingw-cxxabi-1002
                }
            } else:win32-msvc {
                !exists($$BOOST_DIR/lib-mswindows-msvc-8.0-cxxabi-1400) {
                    warning("The boost libraries are not unpacked. See the dir $${BOOST_DIR}.")
                    # remove our local include path
                    INCLUDEPATH -= $$BOOST_DIR/include
                } else {
                    LIBPATH += $$BOOST_DIR/lib-mswindows-msvc-8.0-cxxabi-1400
                }
            } else {
                warning("We do not have boost libs built for your compiler. Make sure you have them installed.")
                # remove our local include path
                INCLUDEPATH -= $$BOOST_DIR/include
            }
        }
    }
    win32-g++ {
        LIBS += -llibboost_signals-mgw-mt-1_33_1
        #FIXME: is there any reason not to use the following instead?
        #LIBS += -lboost_signals-mgw-mt-1_33_1
    } else {
        #LIBS += # we can use auto linking with most other compilers
    }
    
    INCLUDEPATH += $(QTDIR)/include
    INCLUDEPATH += $(QTDIR)/include/Qt
    INCLUDEPATH += $(QTDIR)/include/QtCore
    INCLUDEPATH += $(QTDIR)/include/QtGui
    INCLUDEPATH += $(QTDIR)/include/QtXml
    INCLUDEPATH += $(QTDIR)/src/3rdparty/zlib

    DSOUND_DIR = $$EXTERNAL_PKG_DIR/dsound-9
    !exists($$DSOUND_DIR) {
        warning("The local dsound dir does not exist: $${DSOUND_DIR}. Make sure you have the dsound lib installed.")
        !CONFIG(dsound) {
            message("Assuming you do not have dsound lib. Call qmake CONFIG+=dsound to enable dsound support.")
        }
    } else {
        CONFIG += dsound
        INCLUDEPATH += $$DSOUND_DIR/include
        win32-g++ {
            LIBPATH += $$DSOUND_DIR/lib-mswindows-mingw-cxxabi-1002
        } else {
            LIBPATH += $$DSOUND_DIR/lib-mswindows-msvc-cxxabi
        }
    }
    CONFIG(dsound) {
        win32-g++ {
            LIBS *= -ldsound
            LIBS *= -lwinmm # is this one needed?
            LIBS *= -luuid
        } else {
            LIBS *= dsound.lib
            LIBS *= winmm.lib # is this one needed?
            LIBS *= uuid.lib
        }
        DEFINES += PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE # This is used in the source to determine when to include direct-sound-specific things.
        HEADERS += audiodrivers/microsoftdirectsoundout.h
        SOURCES += audiodrivers/microsoftdirectsoundout.cpp
    }
    
    true { # FIXME: not sure how to test for mme...
        message( "Assuming you have microsoft mme." )
        win32-g++ {
            LIBS *= -lwinmm # is this one needed?
            LIBS *= -luuid
        } else {
            LIBS *= winmm.lib # is this one needed?
            LIBS *= uuid.lib
        }
        DEFINES += PSYCLE__MICROSOFT_MME_AVAILABLE # This is used in the source to determine when to include mme-specific things.
        HEADERS += audiodrivers/microsoftmmewaveout.h
        SOURCES += audiodrivers/microsoftmmewaveout.cpp
    }

    false { # FIXME: asio needs to be built as a lib, which is rather cubersome, or embeeded into qpsycle itself, which sucks...
        message( "Blergh... steinberg asio." )
        win32-g++ {
            LIBS *= -lasio
        } else {
            LIBS *= asio.lib
        }
        DEFINES += PSYCLE__STEINBERG_ASIO_AVAILABLE # This is used in the source to determine when to include asio-specific things.
        HEADERS += audiodrivers/steinbergasioout.h
        SOURCES += audiodrivers/steinbergasioout.cpp
    }
}
