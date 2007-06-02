win32 {
	message("System is: win32.")
	TOP_SRC_DIR=$$system(cd .. && cd)
	win32-g++ {
		message("Compiler is: g++.")
	} else:win32-msvc* {
		message("Compiler is: MS Visual C++.")
		QMAKE_LFLAGS += /PDB:"release\$${QMAKE_TARGET}.pdb"
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
}
