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

HEADERS += \
	src/psycle/player/configuration.h
	
SOURCES += \
	src/psycle/player/main.cpp \
	src/psycle/player/configuration.cpp

include(platform.pri)
include(psycle-core.pri)
include(psycle-audio-drivers.pri)
