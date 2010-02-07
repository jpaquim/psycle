TARGET = psycle-player
TEMPLATE = app # This project builds an executable program.

# include the base stuff shared amongst all qmake projects.
include(../../build-systems/qmake/common.pri)

# this include defines a dependency on the psycle-core lib.
include($$TOP_SRC_DIR/psycle-core/qmake/psycle-core.pri)

# remove default qmake/qt stuff we don't use
CONFIG -= qt uic lex yacc

PSYCLE_PLAYER_DIR = $$TOP_SRC_DIR/psycle-player

BUILD_DIR = $$PSYCLE_PLAYER_DIR/++build
OBJECTS_DIR = $$BUILD_DIR # Where the .o files go.
MOC_DIR = $$BUILD_DIR # Where intermediate moc files go.
RCC_DIR = $$BUILD_DIR # Where intermediate resource files go.
DESTDIR = $$BUILD_DIR # Where the final executable goes.

INCLUDEPATH *= $$PSYCLE_PLAYER_DIR/src
DEPENDPATH  *= $$PSYCLE_PLAYER_DIR/src

#CONFIG *= precompile_header
#PRECOMPILED_HEADER = $$PSYCLE_PLAYER_DIR/src/psycle/player/psyclePlayerPch.hpp

SOURCES_PRESERVE_PATH += \
	$$files($$PSYCLE_PLAYER_DIR/src/psycle/player/*.cpp)
HEADERS += \
	$$files($$PSYCLE_PLAYER_DIR/src/psycle/player/*.hpp)

false: win32 {
	RC_FILE = $$QPSYCLE_DIR/src/psycle-player.rc 
	message("Adding $$RC_FILE for executable icon")
}

include($$COMMON_DIR/display-vars.pri)
