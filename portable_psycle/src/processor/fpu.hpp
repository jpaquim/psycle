///\file
///\brief interface file for processor::fpu
#pragma once
#include <cfloat>
#include <stdexcept>
namespace processor
{
	/// name processor's floating point unit
	namespace fpu
	{
		/// fpu exception mask
		class exception_mask
		{
			#if defined COMPILER__MICROSOFT
				// see the optimization section for #pragma optimize("p", off)
			#endif
			public:
				class type
				{
					private:
						typedef
							#if defined PROCESSOR__X86
								unsigned int
							#else
								#error todo
							#endif
							underlying;
						underlying value;
					public:
						inline operator underlying () const throw() { return value; }
						inline type(underlying const & value) : value(value) {}
						/// by default, inexact and underflow exceptions are masked.
						inline type() : value() { inexact(true); underflow(true); }
						bool inline denormal() const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return value & _EM_DENORMAL;
							#else
								#error todo
							#endif
						}
						void inline denormal(bool b) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								value &= ~_EM_DENORMAL; if(b) value |= _EM_DENORMAL;
							#else
								#error todo
							#endif
						}
						bool inline inexact() const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return value & _EM_INEXACT;
							#else
								#error todo
							#endif
						}
						void inline inexact(bool b) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								value &= ~_EM_INEXACT; if(b) value |= _EM_INEXACT;
							#else
								#error todo
							#endif
						}
						bool inline divide_by_0() const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return value & _EM_ZERODIVIDE;
							#else
								#error todo
							#endif
						}
						void inline divide_by_0(bool b = false) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								value &= ~_EM_ZERODIVIDE; if(b) value |= _EM_ZERODIVIDE;
							#else
								#error todo
							#endif
						}
						bool inline overflow() const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return value & _EM_OVERFLOW;
							#else
								#error todo
							#endif
						}
						void inline overflow(bool b = false) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								value &= ~_EM_OVERFLOW; if(b) value |= _EM_OVERFLOW;
							#else
								#error todo
							#endif
						}
						bool inline underflow() const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return value & _EM_UNDERFLOW;
							#else
								#error todo
							#endif
						}
						void inline underflow(bool b = false) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								value &= ~_EM_UNDERFLOW; if(b) value |= _EM_UNDERFLOW;
							#else
								#error todo
							#endif
						}
						bool inline invalid() const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return value & _EM_INVALID;
							#else
								#error todo
							#endif
						}
						void inline invalid(bool b = false) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								value &= ~_EM_INVALID; if(b) value |= _EM_INVALID;
							#else
								#error todo
							#endif
						}
				};

				type static inline current() throw()
				{
					#if defined PROCESSOR__X86 && defined OPERATING_SYSTEM__MICROSOFT
						return ::_control87(0, 0);
					#else
						#error todo
					#endif
				}

				inline exception_mask(type const & mask)
					#if defined PROCESSOR__X86 && defined OPERATING_SYSTEM__MICROSOFT
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
						#error todo
					#endif
				{}

				inline ~exception_mask() throw()
				{
					#if defined PROCESSOR__X86 && defined OPERATING_SYSTEM__MICROSOFT
						::_control87(save, ~0U);
					#else
						#error todo
					#endif
				}

			private:
				type const save;
		};
	}
}
