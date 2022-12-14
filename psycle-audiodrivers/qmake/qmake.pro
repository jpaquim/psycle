TARGET = psycle-audiodrivers
TEMPLATE = lib # This project builds a library.

# include the base stuff shared amongst all qmake projects.
include(../../build-systems/qmake/common.pri)

include(psycle-audiodrivers.pri)

!CONFIG(shared): CONFIG *= staticlib # Note: Since shared is in CONFIG by default, you will need to pass CONFIG-=shared on qmake's command line to build a static archive.
CONFIG *= create_prl

# remove default qmake/qt stuff we don't use
CONFIG -= qt uic lex yacc

BUILD_DIR = $$PSYCLE_AUDIODRIVERS_BUILD_DIR
OBJECTS_DIR = $$BUILD_DIR # Where the .o files go.
MOC_DIR = $$BUILD_DIR # Where intermediate moc files go.
DESTDIR = $$BUILD_DIR # Where the final executable goes.

CONFIG *= precompile_header
PRECOMPILED_HEADER = $$TOP_SRC_DIR/build-systems/src/forced-include.private.hpp

sources_or_headers = \
	$$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/audiodriver \
	$$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/wavefileout

unix {
	contains(DEFINES, PSYCLE__GSTREAMER_AVAILABLE) {
		sources_or_headers += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/gstreamerout
	}
	contains(DEFINES, PSYCLE__ALSA_AVAILABLE) {
		sources_or_headers += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/alsaout
	}
	contains(DEFINES, PSYCLE__JACK_AVAILABLE) {
		sources_or_headers += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/jackout
	}
	contains(DEFINES, PSYCLE__ESOUND_AVAILABLE) {
		sources_or_headers += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/esoundout
	}
	contains(DEFINES, PSYCLE__NET_AUDIO_AVAILABLE) {
		LIBS *= $$linkLibs(audio)
		sources_or_headers += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/netaudioout
	}
} else: win32 {
	contains(DEFINES, PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE) {
		LIBS *= $$linkLibs(dsound uuid winmm) # is this last one needed?
		sources_or_headers += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/microsoftdirectsoundout
	}
	contains(DEFINES, PSYCLE__MICROSOFT_MME_AVAILABLE) {
		LIBS *= $$linkLibs(winmm uuid) # is this last one needed?
		sources_or_headers += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/microsoftmmewaveout
	}
	contains(DEFINES, PSYCLE__MICROSOFT_ASIO_AVAILABLE) {
		LIBS *= $$linkLibs(asio)
		sources_or_headers += $$PSYCLE_AUDIODRIVERS_DIR/src/psycle/audiodrivers/steinbergasioout
	}
}

SOURCES_PRESERVE_PATH += $$sources(sources_or_headers)
HEADERS += $$headers(sources_or_headers)

include($$COMMON_DIR/display-vars.pri)
