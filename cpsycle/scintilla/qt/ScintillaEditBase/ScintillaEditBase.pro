#-------------------------------------------------
#
# Project created by QtCreator 2011-05-05T12:41:23
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScintillaEditBase
TEMPLATE = lib
CONFIG += lib_bundle
CONFIG += c++1z

VERSION = 5.0.2

SOURCES += \
    PlatQt.cpp \
    ScintillaQt.cpp \
    ScintillaEditBase.cpp \
    ../../src/XPM.cxx \
    ../../src/ViewStyle.cxx \
    ../../src/UniqueString.cxx \
    ../../src/UniConversion.cxx \
    ../../src/Style.cxx \
    ../../src/Selection.cxx \
    ../../src/ScintillaBase.cxx \
    ../../src/RunStyles.cxx \
    ../../src/RESearch.cxx \
    ../../src/PositionCache.cxx \
    ../../src/PerLine.cxx \
    ../../src/MarginView.cxx \
    ../../src/LineMarker.cxx \
    ../../src/KeyMap.cxx \
    ../../src/Indicator.cxx \
    ../../src/Geometry.cxx \
    ../../src/EditView.cxx \
    ../../src/Editor.cxx \
    ../../src/EditModel.cxx \
    ../../src/Document.cxx \
    ../../src/Decoration.cxx \
    ../../src/DBCS.cxx \
    ../../src/ContractionState.cxx \
    ../../src/CharClassify.cxx \
    ../../src/CharacterSet.cxx \
    ../../src/CharacterCategory.cxx \
    ../../src/CellBuffer.cxx \
    ../../src/CaseFolder.cxx \
    ../../src/CaseConvert.cxx \
    ../../src/CallTip.cxx \
    ../../src/AutoComplete.cxx

HEADERS  += \
    PlatQt.h \
    ScintillaQt.h \
    ScintillaEditBase.h \
    ../../src/XPM.h \
    ../../src/ViewStyle.h \
    ../../src/UniConversion.h \
    ../../src/Style.h \
    ../../src/SplitVector.h \
    ../../src/Selection.h \
    ../../src/ScintillaBase.h \
    ../../src/RunStyles.h \
    ../../src/RESearch.h \
    ../../src/PositionCache.h \
    ../../src/Platform.h \
    ../../src/PerLine.h \
    ../../src/Partitioning.h \
    ../../src/LineMarker.h \
    ../../src/KeyMap.h \
    ../../src/Indicator.h \
    ../../src/Geometry.h \
    ../../src/FontQuality.h \
    ../../src/Editor.h \
    ../../src/Document.h \
    ../../src/Decoration.h \
    ../../src/ContractionState.h \
    ../../src/CharClassify.h \
    ../../src/CharacterSet.h \
    ../../src/CharacterCategory.h \
    ../../src/CellBuffer.h \
    ../../src/CaseFolder.h \
    ../../src/CaseConvert.h \
    ../../src/CallTip.h \
    ../../src/AutoComplete.h \
    ../../include/Scintilla.h \
    ../../include/ILexer.h

OTHER_FILES +=

INCLUDEPATH += ../../include ../../src

DEFINES += SCINTILLA_QT=1 MAKING_LIBRARY=1 SCI_LEXER=1 _CRT_SECURE_NO_DEPRECATE=1
CONFIG(release, debug|release) {
    DEFINES += NDEBUG=1
}

DESTDIR = ../../bin

macx {
	QMAKE_LFLAGS_SONAME = -Wl,-install_name,@executable_path/../Frameworks/
}
