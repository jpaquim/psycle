TARGET = psycle-core
TEMPLATE = lib # This project builds a library.

# include the base stuff shared amongst all qmake projects.
include(../../build-systems/qmake/common.pri)

include(psycle-core.pri)

!CONFIG(shared): CONFIG *= staticlib # Note: Since shared is in CONFIG by default, you will need to pass CONFIG-=shared on qmake's command line to build a static archive.
CONFIG *= create_prl

# remove default qmake/qt stuff we don't use
CONFIG -= qt uic lex yacc

BUILD_DIR = $$PSYCLE_CORE_BUILD_DIR
OBJECTS_DIR = $$BUILD_DIR # Where the .o files go.
MOC_DIR = $$BUILD_DIR # Where intermediate moc files go.
DESTDIR = $$BUILD_DIR # Where the final executable goes.

CONFIG *= precompile_header
PRECOMPILED_HEADER = $$TOP_SRC_DIR/build-systems/src/forced-include.private.hpp

SOURCES_PRESERVE_PATH += \
	$$files($$PSYCLE_CORE_DIR/src/psycle/core/*.cpp)
HEADERS += \
	$$files($$PSYCLE_CORE_DIR/src/psycle/core/*.hpp) \
	$$files($$PSYCLE_CORE_DIR/src/psycle/core/*.h)

include($$COMMON_DIR/display-vars.pri)
