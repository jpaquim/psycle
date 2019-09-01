# Use qmake to generate your Makefile from this file.
# N.B. dont run qpsycle --project! It will overwrite 
# this file and we don't want that.
include(../psycle-core/qmake/platform.pri)
TEMPLATE = app
TARGET = 
CONFIG += warn_off
OBJECTS_DIR = build # Where the .o files go.
DEPENDPATH += . \
INCLUDEPATH += . \

# Input
HEADERS += mainwindow.h \
           machineview.h \
           machinegui.h \
           configuration.h \
           global.h \
           inputhandler.h
SOURCES += mainwindow.cpp \
           machineview.cpp \
           machinegui.cpp \
           configuration.cpp \
           inputhandler.cpp \
           global.cpp \
           p3b.cpp

RESOURCES += p3b.qrc

QT += xml

include(../psycle-core/qmake/psycle-core.pri)
include(../psycle-audiodrivers/qmake/psycle-audiodrivers.pri)
message("INCLUDEPATH is $$INCLUDEPATH")
message("LIBS are $$LIBS")
message("PKGCONFIG is $$PKGCONFIG")
