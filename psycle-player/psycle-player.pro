# Use qmake to generate your Makefile from this file.
# N.B. dont run qmake --project! It will overwrite 
# this file and we don't want that.

include(../psycle-core/qmake/platform.pri)

TEMPLATE = app # This project builds an executable program.
TARGET = 

CONFIG += thread
!warn_on: CONFIG += warn_off
message( "Config is $${CONFIG}." )

BUILD_DIR = ++build
OBJECTS_DIR = $$BUILD_DIR # Where the .o files go.
MOC_DIR = $$BUILD_DIR # Where intermediate moc files go.
DESTDIR = $$BUILD_DIR # Where the final executable goes.

PSYCLE_PLAYER_DIR = .
INCLUDEPATH *= src
DEPENDPATH *= src

HEADERS += \
	$$PSYCLE_PLAYER_DIR/src/psycle/player/configuration.hpp
	
SOURCES += \
	$$PSYCLE_PLAYER_DIR/src/psycle/player/main.cpp \
	$$PSYCLE_PLAYER_DIR/src/psycle/player/configuration.cpp

include(../psycle-core/qmake/psycle-core.pri)
include(../psycle-audiodrivers/qmake/psycle-audiodrivers.pri)

unix {
	CONFIG *= link_pkgconfig # adds support for pkg-config via the PKG_CONFIG var

	system( pkg-config --exists libxml++-2.6 ) {
		message( "pkg-config thinks libxml++-2.6 libs are available..." )
		PKGCONFIG += libxml++-2.6
		DEFINES += PSYCLE__LIBXMLPP_AVAILABLE # This is used in the source to determine when to include libxml++-specific things.
	}
}
