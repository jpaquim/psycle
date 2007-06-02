PSYCLE_AUDIODRIVERS_DIR = $$TOP_SRC_DIR/../psycle-audiodrivers
INCLUDEPATH *= $$PSYCLE_AUDIODRIVERS_DIR/src
DEPENDPATH *= $$PSYCLE_AUDIODRIVERS_DIR/src

HEADERS += \
	$$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/audiodriver.h \
	$$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/wavefileout.h
	
SOURCES += \
	$$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/audiodriver.cpp \
	$$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/wavefileout.cpp

include(boost.pri)

unix {
    CONFIG *= link_pkgconfig # adds support for pkg-config via the PKG_CONFIG var

    system( pkg-config --exists alsa ) {
        message( "pkg-config thinks alsa libs are available..." )
        PKGCONFIG += alsa 
        DEFINES += PSYCLE__ALSA_AVAILABLE # This is used in the source to determine when to include alsa-specific things.
        HEADERS += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/alsaout.h
        SOURCES += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/alsaout.cpp
    }

    system( pkg-config --exists jack ) {
        message( "pkg-config thinks jack libs are available..." )
        PKGCONFIG += jack 
        DEFINES += PSYCLE__JACK_AVAILABLE # This is used in the source to determine when to include jack-specific things.
        HEADERS += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/jackout.h
        SOURCES += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/jackout.cpp 
    }

    system( pkg-config --exists esound ) {
        message( "pkg-config thinks esound libs are available..." )
        PKGCONFIG += esound
        DEFINES += PSYCLE__ESOUND_AVAILABLE # This is used in the source to determine when to include esound-specific things.
        HEADERS += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/esoundout.h
        SOURCES += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/esoundout.cpp 
    }

    false { # gstreamer output is unfinished
        system( pkg-config --exists gstreamer ) {
            message( "pkg-config thinks gstreamer libs are available..." )
            PKGCONFIG += gstreamer
            DEFINES += PSYCLE__GSTREAMER_AVAILABLE # This is used in the source to determine when to include gstreamer-specific things.
            HEADERS += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/gstreamerout.h
            SOURCES += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/gstreamerout.cpp 
        }
    }

    false { # note: the net audio output driver is probably not (well) polished/tested anyway. esound is a good alternative.
        # FIXME: not sure how to test for netaudio...
        exists(/usr/include/audio/audiolib.h) {
            LIBS += -laudio
            DEFINES += PSYCLE__NET_AUDIO_AVAILABLE # This is used in the source to determine when to include net-audio-specific things.
            HEADERS += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/netaudioout.h
            SOURCES += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/netaudioout.cpp
        }
    }
} else:win32 {
    EXTERNAL_PKG_DIR = $$TOP_SRC_DIR/../external-packages
    
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
        HEADERS += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/microsoftdirectsoundout.h
        SOURCES += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/microsoftdirectsoundout.cpp
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
        HEADERS += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/microsoftmmewaveout.h
        SOURCES += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/microsoftmmewaveout.cpp
    }

    false { # FIXME: asio needs to be built as a lib, which is rather cubersome, or embeeded into qpsycle itself, which sucks...
        message( "Blergh... steinberg asio." )
        win32-g++ {
            LIBS *= -lasio
        } else {
            LIBS *= asio.lib
        }
        DEFINES += PSYCLE__STEINBERG_ASIO_AVAILABLE # This is used in the source to determine when to include asio-specific things.
        HEADERS += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/steinbergasioout.h
        SOURCES += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/steinbergasioout.cpp
    }
}
