// This file is an exact copy of the following file from freepsycle:
// http://bohan.dyndns.org/cgi-bin/archzoom.cgi/psycle@sourceforge.net/psycle--mainline--LATEST--LATEST/src/project.hpp.in
///\file
///\brief project-wide compiler, operating system, and processor specific tweaks.
///\meta generic
#pragma once
#include "configuration.hpp" // the public configuration header
#if !defined COMPILER || !defined OPERATING_SYSTEM || !defined PROCESSOR
	#include "compilers.hpp"
	#include "operating_systems.hpp"
	#include "processors.hpp"
	#include "microsoft/ndebug.hpp"
	#include "stringized.hpp"
	#if !defined COMPILER__RESOURCE // If this is a resource compiler, skips the rest of this file (only relevant on microsoft's operating system).
		//#include "microsoft/warnings.hpp"
		//#include "microsoft/optimizations.hpp"
		// extensions to the language which would become standard
		#include "iso/stdc_secure_lib.hpp"
		#include "pragmatic/override.hpp"
		#include "pragmatic/pragma.hpp"
		#include "pragmatic/thread_local_storage.hpp"
		#include "pragmatic/aligned.hpp"
		#include "pragmatic/packed.hpp"
		#include "pragmatic/asm.hpp"
		#include "iso/restrict.hpp"
		#include "microsoft/assume.hpp"
		#include "microsoft/super.hpp"
		#include "gnu/typeof.hpp"
		#include "pragmatic/calling_convention.hpp"
		#include "pragmatic/finally.hpp"
		#include "pragmatic/hardware_exception.hpp"
		#include "pragmatic/dynamic_link.hpp"
		#include "pragmatic/numeric.hpp"
	#endif
#endif
