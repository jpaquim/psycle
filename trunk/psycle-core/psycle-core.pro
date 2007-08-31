# Use qmake to generate your Makefile from this file.
# N.B. dont run qmake --project! It will overwrite 
# this file and we don't want that.

include(qmake/platform.pri)

TEMPLATE = lib # This project builds a library.
CONFIG *= plugin # This built library is a loadable module.
TARGET = 

CONFIG *= thread
!warn_on: CONFIG *= warn_off
message("Config is $$CONFIG")

BUILD_DIR = ++build
OBJECTS_DIR = $$BUILD_DIR # Where the .o files go.
MOC_DIR = $$BUILD_DIR # Where intermediate moc files go.
DESTDIR = $$BUILD_DIR # Where the final executable goes.

include(qmake/psycle-core.pri)
include(../psycle-audiodrivers/qmake/psycle-audiodrivers.pri)
