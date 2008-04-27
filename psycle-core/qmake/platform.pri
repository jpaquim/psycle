isEmpty(platform_included) {
	platform_included = 1
	verbose: message("platform included")

	defineReplace(linkLibs) {
		list = $$1
		result =
		for(element, list) {
			unix | win3-g++:   result += -l$${element}
			else: win32-msvc*: result += $${element}.lib
		}
		return($$result)
	}

	unix: include(platform-unix.pri)
	else: win32: include(platform-win32.pri)
	
	!CONFIG(nocolor):contains(QMAKE_CXX, g++):contains(QMAKE_LINK, g++) {
		colorgcc = $$TOP_SRC_DIR/packageneric/generic/scons/colorgcc
		exists($$colorgcc) {
			QMAKE_CXX =  $$colorgcc
			QMAKE_LINK = $$colorgcc
		}
	}
}
