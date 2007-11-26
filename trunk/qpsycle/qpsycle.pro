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
	src/gui/mainwindow.h \
	src/model/instrumentsmodel.h \
	src/gui/global.h \
	src/gui/configuration.h \
	src/gui/inputhandler.h \
	src/gui/patternbox.h \
	src/gui/configdlg/audioconfigdlg.h \
	src/gui/configdlg/settingsdlg.h \
	src/gui/samplebrowser.h \
	src/gui/logconsole.h \
	src/gui/machineview/machinegui.h \
	src/gui/machineview/machinetweakdlg.h \
	src/gui/machineview/machineview.h \
	src/gui/machineview/newmachinedlg.h \
	src/gui/machineview/wiregui.h \
	src/gui/machineview/wiredlg.h \
	src/gui/machineview/generatorgui.h \
	src/gui/machineview/mastergui.h \
	src/gui/machineview/effectgui.h \
	src/gui/patternview/patternview.h \
	src/gui/patternview/linenumbercolumn.h \
	src/gui/patternview/trackheader.h \
	src/gui/patternview/patterngrid.h \
	src/gui/patternview/patterndraw.h \
	src/gui/sequencer/sequencerarea.h \
	src/gui/sequencer/sequencerdraw.h \
	src/gui/sequencer/sequenceritem.h \
	src/gui/sequencer/sequencerline.h \
	src/gui/sequencer/sequencerview.h \
	src/gui/sequencer/beatruler.h \
	src/gui/waveview/waveview.h \
	src/gui/waveview/wavedisplay.h \
	src/gui/waveview/waveamp.h
SOURCES += \
	src/qpsycle.cpp \
	src/qpsyclePch.cpp \
	src/model/instrumentsmodel.cpp \
	src/gui/mainwindow.cpp \
	src/gui/global.cpp \
	src/gui/configuration.cpp \
	src/gui/inputhandler.cpp \
	src/gui/patternbox.cpp \
	src/gui/configdlg/audioconfigdlg.cpp \
	src/gui/configdlg/settingsdlg.cpp \
	src/gui/samplebrowser.cpp \
	src/gui/logconsole.cpp \
	src/gui/machineview/machinegui.cpp \
	src/gui/machineview/machinetweakdlg.cpp \
	src/gui/machineview/machineview.cpp \
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
message("INCLUDEPATH is $$INCLUDEPATH")
message("LIBS are $$LIBS")
message("PKGCONFIG is $$PKGCONFIG")
