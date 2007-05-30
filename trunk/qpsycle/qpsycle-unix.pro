	message("System is: unix.")
	CONFIG += link_pkgconfig # adds support for pkg-config via the PKG_CONFIG var
	
	macx {
		#CONFIG += x86 ppc # make universal binaries on mac
		
		# It looks like xcode discards gcc's env vars, so we make sure they get in by copying them into qmake's vars
		env_cpath = $$system(echo $CPATH | sed \'s,:, ,g\')
		env_library_path = $$system(echo $LIBRARY_PATH | sed \'s,:, ,g\')
		for(path, env_cpath): INCLUDEPATH *= $$path
		for(path, env_library_path): LIBPATH *= $$path
		
		# We add fink's dirs to the search paths, which are installed by default to the /sw prefix
		exists(/sw/include): INCLUDEPATH *= /sw/include
		exists(/sw/lib): LIBPATH *= /sw/lib
	}

	macx: LIBS += -lboost_signals-1_33_1
	else: LIBS += -lboost_signals

	system( pkg-config --exists alsa ) {
		message( "pkg-config thinks alsa libs are available..." )
		PKGCONFIG += alsa 
		DEFINES += PSYCLE__ALSA_AVAILABLE # This is used in the source to determine when to include alsa-specific things.
		HEADERS += \
			src/audiodrivers/alsaout.h \
			src/mididrivers/alsaseqin.h 
		SOURCES += \
			src/audiodrivers/alsaout.cpp \
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

