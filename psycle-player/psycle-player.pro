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
DESTDIR = $$BUILD_DIR # Where the final executable goes.

INCLUDEPATH += src
#DEPENDPATH += src

# Input
HEADERS += \
	src/psycle/player/configuration.h \
	src/audiodrivers/audiodriver.h \
	src/audiodrivers/wavefileout.h \
	src/audiodrivers/dsound.h \
	src/audiodrivers/gstreamerout.h \
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
	src/psycle/player/main.cpp \
	src/psycle/player/configuration.cpp \
	src/audiodrivers/audiodriver.cpp \
	src/audiodrivers/wavefileout.cpp \
	src/audiodrivers/gstreamerout.cpp \
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

QT += xml

unix {
    message("System is: unix.")
    CONFIG += link_pkgconfig # adds support for pkg-config via the PKG_CONFIG var

    LIBS += -lboost_signals

    system( pkg-config --exists alsa ) {
        message( "pkg-config thinks alsa libs are available..." )
        PKGCONFIG += alsa 
        DEFINES += PSYCLE__ALSA_AVAILABLE # This is used in the source to determine when to include alsa-specific things.
        HEADERS += src/audiodrivers/alsaout.h \
                   src/mididrivers/alsaseqin.h 
        SOURCES += src/audiodrivers/alsaout.cpp \
                   src/mididrivers/alsaseqin.cpp 
    }

    system( pkg-config --exists jack ) {
        message( "pkg-config thinks jack libs are available..." )
        PKGCONFIG += jack 
        DEFINES += PSYCLE__JACK_AVAILABLE # This is used in the source to determine when to include jack-specific things.
        HEADERS += src/audiodrivers/jackout.h
        SOURCES += src/audiodrivers/jackout.cpp 
    }

    system( pkg-config --exists esound ) {
        message( "pkg-config thinks esound libs are available..." )
        PKGCONFIG += esound
        DEFINES += PSYCLE__ESOUND_AVAILABLE # This is used in the source to determine when to include esound-specific things.
        HEADERS += src/audiodrivers/esoundout.h
        SOURCES += src/audiodrivers/esoundout.cpp 
    }

    false { # gstreamer output is unfinished
        system( pkg-config --exists gstreamer ) {
            message( "pkg-config thinks gstreamer libs are available..." )
            PKGCONFIG += gstreamer
            DEFINES += PSYCLE__GSTREAMER_AVAILABLE # This is used in the source to determine when to include gstreamer-specific things.
            HEADERS += src/audiodrivers/gstreamerout.h
            SOURCES += src/audiodrivers/gstreamerout.cpp 
        }
    }

    false { # note: the net audio output driver is probably not (well) polished/tested anyway. esound is a good alternative.
        # FIXME: not sure how to test for netaudio...
        exists(/usr/include/audio/audiolib.h) {
            LIBS += -laudio
            DEFINES += PSYCLE__NET_AUDIO_AVAILABLE # This is used in the source to determine when to include net-audio-specific things.
            HEADERS += src/audiodrivers/netaudioout.h
            SOURCES += src/audiodrivers/netaudioout.cpp
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
        HEADERS += src/audiodrivers/microsoftdirectsoundout.h
        SOURCES += src/audiodrivers/microsoftdirectsoundout.cpp
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
        HEADERS += src/audiodrivers/microsoftmmewaveout.h
        SOURCES += src/audiodrivers/microsoftmmewaveout.cpp
    }

    false { # FIXME: asio needs to be built as a lib, which is rather cubersome, or embeeded into qpsycle itself, which sucks...
        message( "Blergh... steinberg asio." )
        win32-g++ {
            LIBS *= -lasio
        } else {
            LIBS *= asio.lib
        }
        DEFINES += PSYCLE__STEINBERG_ASIO_AVAILABLE # This is used in the source to determine when to include asio-specific things.
        HEADERS += src/audiodrivers/steinbergasioout.h
        SOURCES += src/audiodrivers/steinbergasioout.cpp
    }
}
