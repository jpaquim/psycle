namespace processor
{
	/// name processor's floating point unit
	namespace fpu
	{
		using ::real;
		inline void set_mode()
		{
			#if defined PROCESSOR__X86
				#if defined OPERATING_SYSTEM__LINUX || OPERATING_SYSTEM__HURD
					; // nothing for now
				#elif defined OPERATING_SYSTEM__MICROSOFT
					::_control87(_RC_CHOP | _PC_53 | _EM_INEXACT | _EM_DENORMAL, _MCW_RC | _MCW_PC | _MCW_EM);
						// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclib/html/_crt__control87.2c_._controlfp.asp
						// rounding mode: toward zero (iso conformance) (_RC_CHOP, _MCW_RC)
						// precision: 53 bits (_PC_53, _MCW_PC)
						// exception mask: (_EM_INEXACT | _EM_DENORMAL, _MCW_EM)
							// big warning: exceptions are enabled when their *mask* flag is *not* set...
							// so, this enables:  _EM_INVALID | _EM_UNDERFLOW | _EM_OVERFLOW | _EM_ZERODIVIDE
							// and this disables: _EM_INEXACT | _EM_DENORMAL
							// [bohan] i don't know why _EM_DENORMAL stays disabled even when not given as parameter, but it's actually what we want.
							// [bohan] actually, it's was because i used _controlfp, which ignores this for compatibility with non x86 processors
				#endif
				#if defined COMPILER__MICROSOFT
					// see the optimization section for #pragma optimize("p", off)
				#endif
			#elif defined PROCESSOR__POWER_PC
				; // nothing for now
			#else
				; // nothing for now
			#endif
		}
	}
}
