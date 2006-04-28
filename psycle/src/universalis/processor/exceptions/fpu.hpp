// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2006 psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::processor::exceptions::fpu
#pragma once
#include <universalis/detail/project.hpp>
#include <cfloat>
namespace universalis
{
	namespace processor
	{
		namespace exceptions
		{
			/// name processor's floating point unit
			namespace fpu
			{
				/// fpu exception status
				class status
				{
					public:
						void static inline clear() throw()
						{
							#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
								::_clearfp();
							#else
								///\todo
							#endif
						}
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
										return true; ///\todo
									#endif
								}
								void inline mask(underlying const & mask, bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										value &= ~mask; if(b) value |= mask;
									#else
										///\todo
									#endif
								}
							public:
								bool inline denormal() const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										return mask(_EM_DENORMAL);
									#else
										return true; ///\todo
									#endif
								}
								void inline denormal(bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										mask(_EM_DENORMAL, b);
									#else
										///\todo
									#endif
								}
								bool inline inexact() const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										return mask(_EM_INEXACT);
									#else
										return true; ///\todo
									#endif
								}
								void inline inexact(bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										mask(_EM_INEXACT, b);
									#else
										///\todo
									#endif
								}
								bool inline divide_by_0() const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										return mask(_EM_ZERODIVIDE);
									#else
										return true; ///\todo
									#endif
								}
								void inline divide_by_0(bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										mask(_EM_ZERODIVIDE, b);
									#else
										///\todo
									#endif
								}
								bool inline overflow() const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										return mask(_EM_OVERFLOW);
									#else
										return true; ///\todo
									#endif
								}
								void inline overflow(bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										mask(_EM_OVERFLOW, b);
									#else
										///\todo
									#endif
								}
								bool inline underflow() const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										return mask(_EM_UNDERFLOW);
									#else
										return true; ///\todo
									#endif
								}
								void inline underflow(bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										mask(_EM_UNDERFLOW, b);
									#else
										///\todo
									#endif
								}
								bool inline invalid() const throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										return mask(_EM_INVALID);
									#else
										return true; ///\todo
									#endif
								}
								void inline invalid(bool b) throw()
								{
									#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
										mask(_EM_INVALID, b);
									#else
										///\todo
									#endif
								}
						};

						type static inline current() throw()
						{
							#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
								return ::_control87(0, 0);
							#else
								return type(); ///\todo
							#endif
						}

						inline mask(type const & mask)
							#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
							:
								save
								(
									::_control87
									(
										    _RC_CHOP |     _PC_53  |       mask,
										_MCW_RC      | _MCW_PC     | _MCW_EM
									)
									// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt__control87.2c_._controlfp.asp
									// rounding mode: toward zero (iso conformance) (_RC_CHOP, _MCW_RC)
									// precision: 53 bits (_PC_53, _MCW_PC)
									// note: exceptions are enabled when their *mask* flag is *not* set.
									// [bohan] note: ::_controlfp ignores _EM_DENORMAL for compatibility with non x86 processors, like alpha.
								)
							#else
								///\todo
							#endif
						{
						}

						inline ~mask() throw()
						{
							#if defined DIVERSALIS__PROCESSOR__X86 && defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
								::_control87(save, ~type());
							#else
								///\todo
							#endif
						}

					private:
						type const save;
				};
			}
		}
	}
}
