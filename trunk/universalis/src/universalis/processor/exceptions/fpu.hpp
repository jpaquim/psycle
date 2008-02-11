// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\file
///\interface universalis::processor::exceptions::fpu
#pragma once
#include <universalis/detail/project.hpp>
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
	#include <cfloat> // for _control*, _status* and _clear* functions
	#if defined DIVERSALIS__PROCESSOR__X86 // #if DIVERSALIS__PROCESSOR__X86__SSE >= 2
		///\todo use _*87_2 functions
	#endif
#elif __STDC_VERSION__ >= 199901
	#include <fenv.h>
#endif
namespace universalis
{
	namespace processor
	{
		namespace exceptions
		{
			/// processor's floating point unit
			namespace fpu
			{
				/// fpu exception status
				class status
				{
					public:
						void static inline clear() throw()
						{
							#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
								::_clearfp();
								//::_clear87();
							#else
								///\todo
								// C1999 #include <fenv.h>
							#endif
						}

						///\todo function to get current status
				};

				/// fpu exception mask
				class mask
				{
					public:
						class type
						{
							private:
								typedef
									#if defined DIVERSALIS__PROCESSOR__X86
										unsigned int // or a bit field?
									#else
										unsigned int ///\todo
									#endif
									underlying;
									underlying value;
							public:
								inline operator underlying () const throw() { return value; }
								inline type(underlying const & value) : value(value) {}
								/// by default, inexact and underflow exceptions are masked.
								inline type() : value() { inexact(true); underflow(true); }

							private:
								bool inline mask(underlying const & mask) const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										return value & mask;
									#else
										return true; ///\todo C1999 #include <fenv.h>
									#endif
								}
								void inline mask(underlying const & mask, bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										value &= ~mask; if(b) value |= mask;
									#else
										///\todo C1999 #include <fenv.h>
									#endif
								}
							public:
								bool inline denormal() const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
										return mask(_EM_DENORMAL);
									#else
										return true; ///\todo C1999 #include <fenv.h>
									#endif
								}
								void inline denormal(bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
										mask(_EM_DENORMAL, b);
									#else
										///\todo C1999 #include <fenv.h>
									#endif
								}
								bool inline inexact() const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
										return mask(_EM_INEXACT);
									#else
										return true; ///\todo C1999 #include <fenv.h>
									#endif
								}
								void inline inexact(bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
										mask(_EM_INEXACT, b);
									#else
										///\todo C1999 #include <fenv.h>
									#endif
								}
								bool inline divide_by_0() const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
										return mask(_EM_ZERODIVIDE);
									#else
										return true; ///\todo C1999 #include <fenv.h>
									#endif
								}
								void inline divide_by_0(bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
										mask(_EM_ZERODIVIDE, b);
									#else
										///\todo
									#endif
								}
								bool inline overflow() const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
										return mask(_EM_OVERFLOW);
									#else
										return true; ///\todo C1999 #include <fenv.h>
									#endif
								}
								void inline overflow(bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
										mask(_EM_OVERFLOW, b);
									#else
										///\todo C1999 #include <fenv.h>
									#endif
								}
								bool inline underflow() const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
										return mask(_EM_UNDERFLOW);
									#else
										return true; ///\todo C1999 #include <fenv.h>
									#endif
								}
								void inline underflow(bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
										mask(_EM_UNDERFLOW, b);
									#else
										///\todo C1999 #include <fenv.h>
									#endif
								}
								bool inline invalid() const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
										return mask(_EM_INVALID);
									#else
										return true; ///\todo C1999 #include <fenv.h>
									#endif
								}
								void inline invalid(bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
										mask(_EM_INVALID, b);
									#else
										///\todo C1999 #include <fenv.h>
									#endif
								}
						};

						/// the exception mask currently active
						type static inline current() throw()
						{
							#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
								return ::_control87(0, 0);
							#else
								///\todo
								//::_controlfp_s
								//::_controlfp2
								//::_controlfp
								// C1999 #include <fenv.h>
								return type();
							#endif
						}

						/// begins a scoped mask
						inline mask(type const & mask)
							#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
							:
								save //\todo check the doc here. this might not return the previous control flags. ::_control87(0, 0) does it.
								(
									::_control87
									(
										///\todo don't set the rounding mode and precision here
										_RC_CHOP /* rounding mode to set  */ | _PC_53  /* precision to set  */ | mask    /* exception mask to set  */,
										_MCW_RC  /* set the rounding mode */ | _MCW_PC /* set the precision */ | _MCW_EM /* set the exception mask */
									)
									// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt__control87.2c_._controlfp.asp
									// rounding mode: toward zero (iso conformance) (_RC_CHOP, _MCW_RC)
									// precision: 53 bits (_PC_53, _MCW_PC)
									// note: exceptions are enabled when their *mask* flag is *not* set.
									// [bohan] note: ::_controlfp ignores _EM_DENORMAL for compatibility with non x86 processors, like alpha.
								)
							#else
								///\todo
								//::_controlfp_s
								//::_controlfp2
								//::_controlfp
								// C1999 #include <fenv.h>
							#endif
						{
						}

						/// ends a scoped mask
						inline ~mask() throw()
						{
							#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT && defined DIVERSALIS__COMPILER__MICROSOFT
								::_control87
								(
									save, // values to set the control flags to
									~type() // mask indicating which control flags to set
									///\todo don't set back the rounding mode and precision here (pass _MCW_EM)
								);
							#else
								///\todo
								//::_controlfp_s
								//::_controlfp2
								//::_controlfp
								// C1999 #include <fenv.h>
							#endif
						}

					private:
						type const save;
				};
			}
		}
	}
}
