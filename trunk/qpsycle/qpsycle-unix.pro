	message("System is: unix.")
	CONFIG += link_pkgconfig # adds support for pkg-config via the PKG_CONFIG var

	LIBS += -lboost_signals

	system( pkg-config --exists alsa ) {
		message( "pkg-config thinks alsa libs are available..." )
		PKGCONFIG += alsa 
		DEFINES += PSYCLE__ALSA_AVAILABLE # This is used in the source to determine when to include alsa-specific things.
		HEADERS += \
			audiodrivers/alsaout.h \
			mididrivers/alsaseqin.h 
		SOURCES += \
			audiodrivers/alsaout.cpp \
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

