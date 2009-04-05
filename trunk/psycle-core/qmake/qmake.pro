TARGET = psycle-core

# include the base stuff shared amongst all qmake projects.
include(../../packageneric/qmake/common.pri)

include(psycle-core.pri)

TEMPLATE = lib # This project builds a library.
!CONFIG(shared): CONFIG *= staticlib # Note: Since shared is in CONFIG by default, you will need to pass CONFIG-=shared on qmake's command line to build a static archive.
CONFIG *= create_prl

# remove default qmake/qt stuff we don't use
CONFIG -= qt uic lex yacc

BUILD_DIR = $$PSYCLE_CORE_BUILD_DIR
OBJECTS_DIR = $$BUILD_DIR # Where the .o files go.
MOC_DIR = $$BUILD_DIR # Where intermediate moc files go.
DESTDIR = $$BUILD_DIR # Where the final executable goes.

CONFIG *= precompile_header
PRECOMPILED_HEADER = $$TOP_SRC_DIR/packageneric/src/packageneric/pre-compiled.private.hpp

sources_or_headers = \
	$$PSYCLE_CORE_DIR/src/psycle/core/commands \
	$$PSYCLE_CORE_DIR/src/psycle/core/constants \
	$$PSYCLE_CORE_DIR/src/psycle/core/convert_internal_machines \
	$$PSYCLE_CORE_DIR/src/psycle/core/convert_internal_machines.private \
	$$PSYCLE_CORE_DIR/src/psycle/core/cstdint \
	$$PSYCLE_CORE_DIR/src/psycle/core/eventdriver \
	$$PSYCLE_CORE_DIR/src/psycle/core/file \
	$$PSYCLE_CORE_DIR/src/psycle/core/fileio \
	$$PSYCLE_CORE_DIR/src/psycle/core/filterhelper \
	$$PSYCLE_CORE_DIR/src/psycle/core/fwd \
	$$PSYCLE_CORE_DIR/src/psycle/core/instpreview \
	$$PSYCLE_CORE_DIR/src/psycle/core/instrument \
	$$PSYCLE_CORE_DIR/src/psycle/core/internal_machines \
	$$PSYCLE_CORE_DIR/src/psycle/core/internalhost \
	$$PSYCLE_CORE_DIR/src/psycle/core/ladspa \
	$$PSYCLE_CORE_DIR/src/psycle/core/ladspahost \
	$$PSYCLE_CORE_DIR/src/psycle/core/ladspamachine \
	$$PSYCLE_CORE_DIR/src/psycle/core/machine \
	$$PSYCLE_CORE_DIR/src/psycle/core/machinefactory \
	$$PSYCLE_CORE_DIR/src/psycle/core/machinehost \
	$$PSYCLE_CORE_DIR/src/psycle/core/machinekey \
	$$PSYCLE_CORE_DIR/src/psycle/core/mixer \
	$$PSYCLE_CORE_DIR/src/psycle/core/nativehost \
	$$PSYCLE_CORE_DIR/src/psycle/core/patternevent \
	$$PSYCLE_CORE_DIR/src/psycle/core/patternsequence \
	$$PSYCLE_CORE_DIR/src/psycle/core/player \
	$$PSYCLE_CORE_DIR/src/psycle/core/playertimeinfo \
	$$PSYCLE_CORE_DIR/src/psycle/core/plugin \
	$$PSYCLE_CORE_DIR/src/psycle/core/plugincatcher \
	$$PSYCLE_CORE_DIR/src/psycle/core/pluginfinder \
	$$PSYCLE_CORE_DIR/src/psycle/core/plugininfo \
	$$PSYCLE_CORE_DIR/src/psycle/core/preset \
	$$PSYCLE_CORE_DIR/src/psycle/core/psy2filter \
	$$PSYCLE_CORE_DIR/src/psycle/core/psy3filter \
	$$PSYCLE_CORE_DIR/src/psycle/core/psyfilterbase \
	$$PSYCLE_CORE_DIR/src/psycle/core/sampler \
	$$PSYCLE_CORE_DIR/src/psycle/core/sequencer \
	$$PSYCLE_CORE_DIR/src/psycle/core/signalslib \
	$$PSYCLE_CORE_DIR/src/psycle/core/singlepattern \
	$$PSYCLE_CORE_DIR/src/psycle/core/sequencer \
	$$PSYCLE_CORE_DIR/src/psycle/core/song \
	$$PSYCLE_CORE_DIR/src/psycle/core/songserializer \
	$$PSYCLE_CORE_DIR/src/psycle/core/timesignature \
	$$PSYCLE_CORE_DIR/src/psycle/core/vsthost \
	$$PSYCLE_CORE_DIR/src/psycle/core/vstplugin \
	$$PSYCLE_CORE_DIR/src/psycle/core/xminstrument \
	$$PSYCLE_CORE_DIR/src/psycle/core/xml \
	$$PSYCLE_CORE_DIR/src/psycle/core/xmsampler \
	$$PSYCLE_CORE_DIR/src/psycle/core/zipreader \
	$$PSYCLE_CORE_DIR/src/psycle/core/zipwriter \
	$$PSYCLE_CORE_DIR/src/psycle/core/zipwriterstream

SOURCES_PRESERVE_PATH += $$sources(sources_or_headers)
HEADERS += $$headers(sources_or_headers)

include($$COMMON_DIR/display-vars.pri)
