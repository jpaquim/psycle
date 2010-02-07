isEmpty(platform_win32_included) {
	platform_win32_included = 1
	verbose: message("platform-win32 included")

	win32 {
		verbose: message("System is win32")
		
		EXTERNAL_PKG_DIR = $$TOP_SRC_DIR/external-packages
		
		win32-g++ {
			message("Compiler is g++")
			#DEFINES *= NOMINMAX # no problem on mingw if stl algo header is included before the winapi
		} else: win32-msvc* {
			message("Compiler is MS Visual C++")
			QMAKE_CXXFLAGS *= /Zi
			QMAKE_LFLAGS *= /debug
			LIBS *= $$linkLibs(advapi32 user32)
			DEFINES *= NOMINMAX # This stops windows headers from creating min & max as defines (not needed on mingw)

			win32-msvc2008 {
				VC_VERSION = 9.0
				message("Compiler is MS Visual C++ version 15 (2008)")
			}
			else:win32-msvc2005 {
				VC_VERSION = 8.0
				message("Compiler is MS Visual C++ version 14 (2005)")
			} else {
				VC_VERSION = 7.1
			}

			# Question: Do these regiser db entries depend on the IDE Studio being installed?
			VC_DIR = $$system("reg query HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\$$VC_VERSION\Setup\VC /v ProductDir | findstr REG_SZ")
			VC_DIR -= ProductDir  REG_SZ

			#VC_DIR = $$system("reg query HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\$$VC_VERSION /v InstallDir | findstr REG_SZ")
			#VC_DIR -= InstallDir REG_SZ

			exists($$VC_DIR) {
				#VC_DIR = $$system("dir \"$$VC_DIR\..\..\VC\" | findstr Directory")
				#VC_DIR -= Directory of
				message("Existing VC_DIR is $$VC_DIR")
				LIBPATH *= "$${VC_DIR}/lib"
				win32-msvc2005: LIBPATH *= "$${VC_DIR}/PlatformSDK/lib"
				# todo: include Windows SDK Dirs (aren't they automatically included When Using Vc Command Prompt?
				# todo: what about INCLUDEPATH?
			}
		} else {
			warning("Untested compiler")
		}
		
	}
}
