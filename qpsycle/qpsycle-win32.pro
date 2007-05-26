# The horror begins...
	message("System is: win32.")
	win32-g++ {
		message("Compiler is: g++.")
	} else:win32-msvc* {
		message("Compiler is: MS Visual C++.")
		win32-msvc2005 {
			message("Compiler is: MS Visual C++ 14 (2005).")
			# Question: Do these regiser db entries depend on the IDE Studio being installed?
			VC_DIR = $$system("reg query HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\8.0\Setup\VC /v ProductDir|findstr REG_SZ")
			VC_DIR -= ProductDir  REG_SZ

			#VC_DIR = $$system("reg query HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\8.0 /v InstallDir|findstr REG_SZ")
			#VC_DIR -= InstallDir REG_SZ

			exists($${VC_DIR}) {
				#VC_DIR = $$system("dir \"$${VC_DIR}..\..\VC\"|findstr Directory")
				#VC_DIR -= Directory of
				message("Existing VC_DIR is [$${VC_DIR}].")
				LIBPATH *= "$${VC_DIR}/lib"
				LIBPATH *= "$${VC_DIR}/PlatformSDK/lib"
			}
			
			LIBS *= advapi32.lib
			LIBS *= user32.lib
		}
	} else {
		warning("Untested compiler.")
	}

	INCLUDEPATH *= $(QTDIR)/include
	INCLUDEPATH *= $(QTDIR)/include/Qt
	INCLUDEPATH *= $(QTDIR)/include/QtCore
	INCLUDEPATH *= $(QTDIR)/include/QtGui
	INCLUDEPATH *= $(QTDIR)/include/QtXml
	INCLUDEPATH *= $(QTDIR)/src/3rdparty/zlib

	CWD = $$system(cd)
	EXTERNAL_PKG_DIR = $$CWD/../external-packages

	exists($(BOOST_DIR)) {
		message("Existing BOOST_DIR is [$(BOOST_DIR)].")
		BOOST_DIR = $(BOOST_DIR)
		INCLUDEPATH *= $${BOOST_DIR}
		LIBPATH *= $${BOOST_DIR}/lib
	} else {
		BOOST_DIR = $$EXTERNAL_PKG_DIR/boost-1.33.1
		!exists($$BOOST_DIR) {
			warning("The local boost dir does not exist: $${BOOST_DIR}. Make sure you have boost libs installed.")
		} else {
			!exists($$BOOST_DIR)/include {
				warning("The boost headers are not unpacked. See the dir $${BOOST_DIR}.")
			} else {
				INCLUDEPATH += $$BOOST_DIR/include
				win32-g++ {
					!exists($$BOOST_DIR/lib-mswindows-mingw-cxxabi-1002) {
						warning("The boost libraries are not unpacked. See the dir $${BOOST_DIR}.")
						# remove our local include path
						INCLUDEPATH -= $$BOOST_DIR/include
					} else {
						LIBPATH *= $$BOOST_DIR/lib-mswindows-mingw-cxxabi-1002
						LIBS *= -llibboost_signals-mgw-mt-1_33_1
						#FIXME: is there any reason not to use the following instead?
						#LIBS *= -lboost_signals-mgw-mt-1_33_1
					}
				} else:win32-msvc {
					warning("We do not have boost libs built for your compiler. Make sure you have them installed")
					# remove our local include path
					INCLUDEPATH -= $$BOOST_DIR/include
				} else:win32-msvc2005 {
					!exists($$BOOST_DIR/lib-mswindows-msvc-8.0-cxxabi-1400) {
						warning("The boost libraries are not unpacked. See the dir $$BOOST_DIR".)
						# remove our local include path
						INCLUDEPATH -= $$BOOST_DIR/include
					} else {
						LIBPATH *= $$BOOST_DIR/lib-mswindows-msvc-8.0-cxxabi-1400
						LIBS *= boost_signals-vc80-mt-1_33_1.lib
					}
				} else {
					warning("We do not have boost libs built for your compiler. Make sure you have them installed.")
					# remove our local include path
					INCLUDEPATH -= $$BOOST_DIR/include
				}
			}
		}
	}

	exists($(DXSDK_DIR)) {
		message("Existing DXSDK_DIR is [$(DXSDK_DIR)].")
		INCLUDEPATH += $(DXSDK_DIR)/include
		LIBPATH += $(DXSDK_DIR)/lib
		CONFIG += dsound
	} else {
		DSOUND_DIR = $$EXTERNAL_PKG_DIR/dsound-9
		!exists($$DSOUND_DIR) {
			warning("The local dsound dir does not exist: $${DSOUND_DIR}. Make sure you have the dsound lib installed.")
			!CONFIG(dsound) {
				message("Assuming you do not have dsound lib. Call qmake CONFIG+=dsound to enable dsound support.")
			}
		} else {
			CONFIG += dsound
			INCLUDEPATH += $$DSOUND_DIR/include
			win32-g++ {
				LIBPATH += $$DSOUND_DIR/lib-mswindows-mingw-cxxabi-1002
			} else {
				LIBPATH += $$DSOUND_DIR/lib-mswindows-msvc-cxxabi
			}
		}
	}
	CONFIG(dsound) {
		win32-g++ {
			LIBS *= -ldsound
			LIBS *= -luuid
			LIBS *= -lwinmm # is this one needed?
		} else {
			LIBS *= dsound.lib
			LIBS *= uuid.lib
			LIBS *= winmm.lib # is this one needed?
		}
		DEFINES += PSYCLE__MICROSOFT_DIRECT_SOUND_AVAILABLE # This is used in the source to determine when to include direct-sound-specific things.
		HEADERS += audiodrivers/microsoftdirectsoundout.h
		SOURCES += audiodrivers/microsoftdirectsoundout.cpp
	}
	
	true { # FIXME: not sure how to test for mme...
		message( "Assuming you have microsoft mme." )
		win32-g++ {
			LIBS *= -lwinmm
			LIBS *= -luuid # is this one needed?
		} else {
			LIBS *= winmm.lib
			LIBS *= uuid.lib # is this one needed?
		}
		DEFINES += PSYCLE__MICROSOFT_MME_AVAILABLE # This is used in the source to determine when to include mme-specific things.
		HEADERS += audiodrivers/microsoftmmewaveout.h
		SOURCES += audiodrivers/microsoftmmewaveout.cpp
	}

	false { # FIXME: asio needs to be built as a lib, which is rather cubersome, or embeeded into qpsycle itself, which sucks...
		message( "Blergh... steinberg asio." )
		win32-g++ {
			LIBS *= -lasio
		} else {
			LIBS *= asio.lib
		}
		DEFINES += PSYCLE__STEINBERG_ASIO_AVAILABLE # This is used in the source to determine when to include asio-specific things.
		HEADERS += audiodrivers/steinbergasioout.h
		SOURCES += audiodrivers/steinbergasioout.cpp
	}

	win32-msvc* {
		QMAKE_LFLAGS += /PDB:"release\QPsycle.pdb"
	}



