#!/usr/bin/env python3
# LexGen.py - implemented 2002 by Neil Hodgson neilh@scintilla.org
# Released to the public domain.

# Regenerate the Scintilla source files that list all the lexers.
# Should be run whenever a new lexer is added or removed.
# Requires Python 3.6 or later
# Files are regenerated in place with templates stored in comments.
# The format of generation comments is documented in FileGenerator.py.
# Also updates version numbers and modification dates.

from FileGenerator import Regenerate, UpdateLineInFile, \
    ReplaceREInFile, UpdateLineInPlistFile, ReadFileAsList, UpdateFileFromLines, \
    FindSectionInList
import ScintillaData
import HFacer
import os
import pathlib
import uuid
import sys

baseDirectory = os.path.dirname(os.path.dirname(ScintillaData.__file__))
sys.path.append(baseDirectory)

import win32.DepGen
import gtk.DepGen

def UpdateVersionNumbers(sci, root):
    UpdateLineInFile(root / "win32/ScintRes.rc", "#define VERSION_SCINTILLA",
        "#define VERSION_SCINTILLA \"" + sci.versionDotted + "\"")
    UpdateLineInFile(root / "win32/ScintRes.rc", "#define VERSION_WORDS",
        "#define VERSION_WORDS " + sci.versionCommad)
    UpdateLineInFile(root / "qt/ScintillaEditBase/ScintillaEditBase.pro",
        "VERSION =",
        "VERSION = " + sci.versionDotted)
    UpdateLineInFile(root / "qt/ScintillaEdit/ScintillaEdit.pro",
        "VERSION =",
        "VERSION = " + sci.versionDotted)
    UpdateLineInFile(root / "doc/ScintillaDownload.html", "       Release",
        "       Release " + sci.versionDotted)
    ReplaceREInFile(root / "doc/ScintillaDownload.html",
        r"/www.scintilla.org/([a-zA-Z]+)\d\d\d",
        r"/www.scintilla.org/\g<1>" +  sci.version,
        0)
    UpdateLineInFile(root / "doc/index.html",
        '          <font color="#FFCC99" size="3"> Release version',
        '          <font color="#FFCC99" size="3"> Release version ' +\
        sci.versionDotted + '<br />')
    UpdateLineInFile(root / "doc/index.html",
        '           Site last modified',
        '           Site last modified ' + sci.mdyModified + '</font>')
    UpdateLineInFile(root / "doc/ScintillaHistory.html",
        '	Released ',
        '	Released ' + sci.dmyModified + '.')

    cocoa = root / "cocoa"

    UpdateLineInPlistFile(cocoa / "Scintilla" / "Info.plist",
        "CFBundleShortVersionString", sci.versionDotted)
    ReplaceREInFile(cocoa / "Scintilla"/ "Scintilla.xcodeproj" / "project.pbxproj",
        "CURRENT_PROJECT_VERSION = [0-9.]+;",
        f'CURRENT_PROJECT_VERSION = {sci.versionDotted};')

# Last 24 digits of UUID, used for item IDs in Xcode
def uid24():
    return str(uuid.uuid4()).replace("-", "").upper()[-24:]

def ciLexerKey(a):
    return a.split()[2].lower()

def RegenerateAll(rootDirectory):
    
    root = pathlib.Path(rootDirectory)

    scintillaBase = root.resolve()

    sci = ScintillaData.ScintillaData(scintillaBase)

    Regenerate(scintillaBase / "win32/scintilla.mak", "#", sci.lexFiles)

    startDir = os.getcwd()
    os.chdir(os.path.join(scintillaBase, "win32"))
    win32.DepGen.Generate()
    os.chdir(os.path.join(scintillaBase, "gtk"))
    gtk.DepGen.Generate()
    os.chdir(startDir)

    UpdateVersionNumbers(sci, root)

    HFacer.RegenerateAll(root, False)

if __name__=="__main__":
    RegenerateAll(pathlib.Path(__file__).resolve().parent.parent)
