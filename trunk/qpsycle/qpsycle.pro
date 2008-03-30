# Use qmake to generate your Makefile from this file.
# N.B. dont run qmake --project! It will overwrite
# this file and we don't want that.
include(../psycle-core/qmake/platform.pri)
TEMPLATE = app # This project builds an executable program.
TARGET = 
CONFIG *= thread
!warn_on:CONFIG *= warn_off
message("Config is $$CONFIG")
BUILD_DIR = ++build
OBJECTS_DIR = $$BUILD_DIR # Where the .o files go.
MOC_DIR = $$BUILD_DIR # Where intermediate moc files go.
RCC_DIR = $$BUILD_DIR # Where intermediate resource files go.
DESTDIR = $$BUILD_DIR # Where the final executable goes.
DEPENDPATH += . \
	src/ \
	src/model \
	src/gui \
	src/gui/machineview \
	src/gui/patternview \
	src/gui/sequencer \
	src/gui/waveview \
	src/configdlg
INCLUDEPATH += \
	src \
	src/mididrivers \
	src/model \
	src/gui \
	src/gui/machineview \
	src/gui/patternview \
	src/gui/waveview \
	src/gui/sequencer \
	src/gui/configdlg
HEADERS += \
	src/gui/mainwindow.hpp \
	src/model/instrumentsmodel.hpp \
	src/gui/global.hpp \
	src/gui/configuration.hpp \
	src/gui/inputhandler.hpp \
	src/gui/patternbox.hpp \
	src/gui/configdlg/audiopage.hpp \
	src/gui/configdlg/behaviourpage.hpp \
        src/gui/configdlg/configdlg.hpp \
        src/gui/configdlg/lookspage.hpp \
        src/gui/configdlg/dirspage.hpp \
	src/gui/samplebrowser.hpp \
	src/gui/logconsole.hpp \
	src/gui/machineview/machinegui.hpp \
	src/gui/machineview/machinetweakdlg.hpp \
	src/gui/machineview/machineview.hpp \
	src/gui/machineview/mixertweakdlg.hpp \
	src/gui/machineview/newmachinedlg.hpp \
	src/gui/machineview/wiregui.hpp \
	src/gui/machineview/wiredlg.hpp \
	src/gui/machineview/generatorgui.hpp \
	src/gui/machineview/mastergui.hpp \
	src/gui/machineview/effectgui.hpp \
	src/gui/patternview/patternview.hpp \
	src/gui/patternview/linenumbercolumn.hpp \
	src/gui/patternview/trackheader.hpp \
	src/gui/patternview/patterngrid.hpp \
	src/gui/patternview/patterndraw.hpp \
	src/gui/sequencer/sequencerarea.hpp \
	src/gui/sequencer/sequencerdraw.hpp \
	src/gui/sequencer/sequenceritem.hpp \
	src/gui/sequencer/sequencerline.hpp \
	src/gui/sequencer/sequencerview.hpp \
	src/gui/sequencer/beatruler.hpp \
	src/gui/waveview/waveview.hpp \
	src/gui/waveview/wavedisplay.hpp \
	src/gui/waveview/waveamp.hpp
SOURCES += \
	src/qpsycle.cpp \
	src/qpsyclePch.cpp \
	src/model/instrumentsmodel.cpp \
	src/gui/mainwindow.cpp \
	src/gui/global.cpp \
	src/gui/configuration.cpp \
	src/gui/inputhandler.cpp \
	src/gui/patternbox.cpp \
	src/gui/configdlg/audiopage.cpp \
	src/gui/configdlg/behaviourpage.cpp \
        src/gui/configdlg/configdlg.cpp \
        src/gui/configdlg/lookspage.cpp \
        src/gui/configdlg/dirspage.cpp \
	src/gui/samplebrowser.cpp \
	src/gui/logconsole.cpp \
	src/gui/machineview/machinegui.cpp \
	src/gui/machineview/machinetweakdlg.cpp \
	src/gui/machineview/machineview.cpp \
	src/gui/machineview/mixertweakdlg.cpp \ 
	src/gui/machineview/newmachinedlg.cpp \
	src/gui/machineview/wiregui.cpp \
	src/gui/machineview/wiredlg.cpp \
	src/gui/machineview/generatorgui.cpp \
	src/gui/machineview/mastergui.cpp \
	src/gui/machineview/effectgui.cpp \
	src/gui/patternview/patternview.cpp \
	src/gui/patternview/linenumbercolumn.cpp \
	src/gui/patternview/trackheader.cpp \
	src/gui/patternview/patterngrid.cpp \
	src/gui/patternview/patterndraw.cpp \
	src/gui/sequencer/sequencerarea.cpp \
	src/gui/sequencer/sequencerdraw.cpp \
	src/gui/sequencer/sequenceritem.cpp \
	src/gui/sequencer/sequencerline.cpp \
	src/gui/sequencer/sequencerview.cpp \
	src/gui/sequencer/beatruler.cpp \
	src/gui/waveview/waveview.cpp \
	src/gui/waveview/wavedisplay.cpp \
	src/gui/waveview/waveamp.cpp
RESOURCES += src/qpsycle.qrc
win32 {
	RC_FILE = src/qpsycle.rc 
	message("Adding $$RC_FILE for executable icon")
}
include(../psycle-core/qmake/psycle-core.pri)
include(../psycle-audiodrivers/qmake/psycle-audiodrivers.pri)
include(qmake/qt-xml.pri)
message("INCLUDEPATH is $$INCLUDEPATH")
message("LIBS are $$LIBS")
message("PKGCONFIG is $$PKGCONFIG")
