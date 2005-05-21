// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\brief processor's floating point unit
#pragma once
#include "project.hpp"
namespace processor
{
	/// processor's floating point unit
	namespace fpu
	{
		#if !defined COMPILER__ECLIPSE
			using ::real;
		#endif
		inline void set_mode()
		{
			#if defined PROCESSOR__X86
				#if defined OPERATING_SYSTEM__LINUX || OPERATING_SYSTEM__HURD
					// nothing for now
				#elif defined OPERATING_SYSTEM__MICROSOFT
					::_control87(_RC_CHOP | _PC_53 | _EM_INEXACT | _EM_DENORMAL, _MCW_RC | _MCW_PC | _MCW_EM);
						// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt__control87.2c_._controlfp.asp
						// rounding mode: toward zero (iso conformance) (_RC_CHOP, _MCW_RC)
						// precision: 53 bits (_PC_53, _MCW_PC)
						// exception mask: (_EM_INEXACT | _EM_DENORMAL, _MCW_EM)
							// exceptions are enabled when their *mask* flag is *not* set.
							// so, this enables:  _EM_INVALID | _EM_UNDERFLOW | _EM_OVERFLOW | _EM_ZERODIVIDE
							// and this disables: _EM_INEXACT | _EM_DENORMAL
							// <bohan> i don't know why _EM_DENORMAL stays disabled even when not given as parameter, but it's actually what we want.
							// <bohan> actually, it's was because i used ::_controlfp, which ignores this for compatibility with non x86 processors
				#endif
				#if defined COMPILER__MICROSOFT
					// see <operating_system/project.hpp>, the optimization section for #pragma optimize("p", off)
				#endif
			#elif defined PROCESSOR__POWER_PC
				// nothing for now
			#else
				// nothing for now
			#endif
		}
	}
}

// arch-tag: 9a28788e-b32f-4afc-8571-1bce8df658d6
