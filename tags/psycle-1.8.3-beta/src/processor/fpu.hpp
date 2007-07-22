///\file
///\brief interface file for processor::fpu
#pragma once
#include <cfloat>
namespace processor
{
	/// name processor's floating point unit
	namespace fpu
	{
		/// fpu exception status
		class exception_status
		{
			public:
				void static inline clear() throw()
				{
					#if defined OPERATING_SYSTEM__MICROSOFT
						::_clearfp();
					#else
						#error todo
					#endif
				}
		};

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
								unsigned int // or a bit field?
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

					private:
						bool inline mask(underlying const & mask) const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return value & mask;
							#else
								#error todo
							#endif
						}
						void inline mask(underlying const & mask, bool b) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								value &= ~mask; if(b) value |= mask;
							#else
								#error todo
							#endif
						}
					public:
						bool inline denormal() const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return mask(_EM_DENORMAL);
							#else
								#error todo
							#endif
						}
						void inline denormal(bool b) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								mask(_EM_DENORMAL, b);
							#else
								#error todo
							#endif
						}
						bool inline inexact() const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return mask(_EM_INEXACT);
							#else
								#error todo
							#endif
						}
						void inline inexact(bool b) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								mask(_EM_INEXACT, b);
							#else
								#error todo
							#endif
						}
						bool inline divide_by_0() const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return mask(_EM_ZERODIVIDE);
							#else
								#error todo
							#endif
						}
						void inline divide_by_0(bool b) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								mask(_EM_ZERODIVIDE, b);
							#else
								#error todo
							#endif
						}
						bool inline overflow() const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return mask(_EM_OVERFLOW);
							#else
								#error todo
							#endif
						}
						void inline overflow(bool b) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								mask(_EM_OVERFLOW, b);
							#else
								#error todo
							#endif
						}
						bool inline underflow() const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return mask(_EM_UNDERFLOW);
							#else
								#error todo
							#endif
						}
						void inline underflow(bool b) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								mask(_EM_UNDERFLOW, b);
							#else
								#error todo
							#endif
						}
						bool inline invalid() const throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								return mask(_EM_INVALID);
							#else
								#error todo
							#endif
						}
						void inline invalid(bool b) throw()
						{
							#if defined OPERATING_SYSTEM__MICROSOFT
								mask(_EM_INVALID, b);
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
