#-------------------------------------------------
#
# Project created by QtCreator 2012-05-25T14:03:20
#
#-------------------------------------------------

TEMPLATE = subdirs

QMAKE_CXXFLAGS += -std=c++11

include(../build-systems/qmake/common.pri)
addSubdirs(../psycle-core)
addSubdirs(qmake, ../psycle-core)

#TARGET = qpsycle2

#TEMPLATE += app

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#QPSYCLE_2_DIR = $$TOP_SRC_DIR/qpsycle2

#BUILD_DIR = $$QPSYCLE_2_DIR/++qmake
#OBJECTS_DIR = $$BUILD_DIR # Where the .o files go.
#MOC_DIR = $$BUILD_DIR # Where intermediate moc files go.
#RCC_DIR = $$BUILD_DIR # Where intermediate resource files go.
#DESTDIR = $$BUILD_DIR # Where the final executable goes.



#HEADERS  += src/qpsycle2.h \
#    src/menus/filemenu.h \
#    src/menus/editmenu.h \
#    src/menus/menusignalhandler.h \
#    src/menus/toolbar.h \
#    src/MachineView/machineviewline.h \
#    src/MachineView/machineviewguielement.h \
#    src/MachineView/machineview.h \
#    src/MachineView/guimachineelement.h \
#    src/MachineView/machinethemeloader.h \
#    src/MachineView/generatorguielement.h \
#    src/MachineView/mixerguielement.h \
#    src/MachineView/effectguielement.h \
#    src/MachineView/newmachinedialog.h \
#    src/statics.h \
#    src/MachineView/machinetweakdialog.h \
#    src/MachineView/machineviewwiredialog.h \
##    src/PatternView/patternview.h \
##    src/PatternView/patternviewpattern.h \
##    src/PatternView/patternviewtrack.h \
##    src/PatternView/patternviewtrackheader.h

#SOURCES += src/main.cpp\
#    src/qpsycle2.cpp \
#    src/menus/filemenu.cpp \
#    src/menus/editmenu.cpp \
#    src/menus/menusignalhandler.cpp \
#    src/menus/toolbar.cpp \
#    src/MachineView/machineviewline.cpp \
#    src/MachineView/machineview.cpp \
#    src/MachineView/guimachineelement.cpp \
#    src/MachineView/machinethemeloader.cpp \
#    src/MachineView/generatorguielement.cpp \
#    src/MachineView/mixerguielement.cpp \
#    src/MachineView/effectguielement.cpp \
#    src/MachineView/newmachinedialog.cpp \
#    src/statics.cpp \
#    src/MachineView/machinetweakdialog.cpp \
#    src/MachineView/machineviewwiredialog.cpp \
##    src/PatternView/patternview.cpp \
##    src/PatternView/patternviewpattern.cpp \
##    src/PatternView/patternviewtrack.cpp \
##    src/PatternView/patternviewtrackheader.cpp

#INCLUDEPATH += ../universalis/src/\
#     ../diversalis/src/\
#     ../psycle-helpers/src/\
#     ../psycle-plugins/src/\
#     ../psycle/src/Seib-vsthost/\
#     ../psycle/src/\
#     ../external-packages/vst-2.4/\
#     ../psycle-core/src/\
#     ../psycle-audiodrivers/src/\
#     src/\


# LIBS += -lz -ldl -lrt -lboost_filesystem -lboost_system -lboost_signals -lboost_thread

#OTHER_FILES +=

#CONFIG      += no_keywords\
#            link_pkgconfig\

# PKGCONFIG += glib-2.0 gstreamer-0.10 gstreamer-plugins-base-0.10 alsa jack soxr


#QMAKE_CXXFLAGS += -std=c++11

#QMAKE_CFLAGS_DEBUG += -O0

#DEFINES += PSYCLE__ALSA_AVAILABLE
#DEFINES += PSYCLE__GSTREAMER_AVAILABLE
#DEFINES += PSYCLE__JACK_AVAILABLE

#BUILD_DIR = ++build
#OBJECTS_DIR = $$BUILD_DIR/objects # Where the .o files go.
#MOC_DIR = $$BUILD_DIR/moc # Where intermediate moc files go.
#RCC_DIR = $$BUILD_DIR/rcc # Where intermediate resource files go.
#UI_DIR = $$BUILD_DIR/uic # Where compiled uic files go.
#DESTDIR = $$BUILD_DIR # Where the final executable goes.


#include(../psycle-core/psycle-core.pro)
#include(../psycle-audiodrivers/psycle-audiodrivers.pro)
