// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2000 Ross Bencina and Phil Burk
// copyright ????-???? Bjorn Roche, XO Audio, LLC
// copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>
//
// This source code is based on the pa_memorybarrier.h header of PortAudio (Portable Audio I/O Library)
// For more information on PortAudio, see: http://www.portaudio.com
//
// The PortAudio community makes the following non-binding requests:
// Any person wishing to distribute modifications to the Software
// is requested to send the modifications to the original developer
// so that they can be incorporated into the canonical version.
// It is also requested that these non-binding requests be included along with the license above.

///\file \brief memory barrier utilities
/// Some memory barrier primitives based on the system.
/// In addition to providing memory barriers, these functions should ensure that data cached in registers
/// is written out to cache where it can be snooped by other CPUs. (ie, the volatile keyword should not be required).
///
/// the primitives that defined here are:
///
/// universalis::processor::memory_barriers::full()
/// universalis::processor::memory_barriers::read()
/// universalis::processor::memory_barriers::write()

#include <diversalis/diversalis.hpp>
#if defined DIVERSALIS__OPERATING_SYSTEM__APPLE
	#include <libkern/OSAtomic.h>
#elif defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION >= 1400
	#include <intrin.h>
#endif

namespace universalis { namespace processor { namespace memory_barriers {
	#if defined DIVERSALIS__OPERATING_SYSTEM__APPLE
		// Here are the memory barrier functions. Mac OS X only provides
		// full memory barriers, so the three types of barriers are the same,
		// however, these barriers are superior to compiler-based ones.

		void inline  full() { OSMemoryBarrier(); }
		void inline  read() { full(); }
		void inline write() { full(); }
	#elif defined DIVERSALIS__COMPILER__GNU
		#if DIVERSALIS__COMPILER__VERSION >= 40100 // 4.1.0
			// GCC >= 4.1 has built-in intrinsics. We'll use those.
			void inline  full() { __sync_synchronize(); }
			void inline  read() { full(); }
			void inline write() { full(); }
		#else
			// As a fallback, GCC understands volatile asm and "memory"
			// to mean it should not reorder memory read/writes.
			#if defined DIVERSALIS__PROCESSOR__POWER_PC // defined  __ppc__ || defined __powerpc__
				void inline  full() { asm volatile("sync":::"memory"); }
				void inline  read() { full(); }
				void inline write() { full(); }
			#elif defined DIVERSALIS__PROCESSOR__X86 // defined __i386__ || defined __i486__ || defined __i586__ || defined __i686__ || defined __x86_64__
				void inline  full() { asm volatile("mfence":::"memory"); }
				void inline  read() { asm volatile("lfence":::"memory"); }
				void inline write() { asm volatile("sfence":::"memory"); }
			#else
				#warning Memory barriers are not defined on this system. SMP safety will not be guaranteed.
				void inline  full() {}
				void inline  read() {}
				void inline write() {}
			#endif
		#endif
	#elif defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION >= 1400
		#pragma intrinsic(_ReadWriteBarrier)
		#pragma intrinsic(_ReadBarrier)
		#pragma intrinsic(_WriteBarrier)
		void inline  full() { _ReadWriteBarrier(); }
		void inline  read() { _ReadBarrier(); }
		void inline write() { _WriteBarrier(); }
	#elif defined DIVERSALIS__COMPILER__MICROSOFT || defined DIVERSALIS__COMPILER__BORLAND
		void inline  full() { _asm { lock add [esp], 0}; }
		void inline  read() { full(); }
		void inline write() { full(); }
	#else
		#warning Memory barriers are not defined on this system. SMP safety will not be guaranteed.
		void inline  full() {}
		void inline  read() {}
		void inline write() {}
	#endif
}}}
