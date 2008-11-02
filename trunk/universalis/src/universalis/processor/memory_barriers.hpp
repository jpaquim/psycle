// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2000 Ross Bencina and Phil Burk
// copyright ????-???? Bjorn Roche, XO Audio, LLC
// copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>
//
// This source code is based on the pa_memorybarrier.h header of PortAudio (Portable Audio I/O Library)
// See portaudio memory barrier http://portaudio.com/trac/browser/portaudio/trunk/src/common/pa_memorybarrier.h
// For more information on PortAudio, see http://www.portaudio.com
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
/// The primitives defined here are:
/// universalis::processor::memory_barriers::full()
/// universalis::processor::memory_barriers::read()
/// universalis::processor::memory_barriers::write()

#include <diversalis/diversalis.hpp>

#if defined DIVERSALIS__OPERATING_SYSTEM__APPLE
	#include <libkern/OSAtomic.h>
	// Mac OS X only provides full memory barriers, so the three types of barriers are the same,
	// However, these barriers are superior to compiler-based ones.
	namespace universalis { namespace processor { namespace memory_barriers {
		void inline  full() { OSMemoryBarrier(); }
		void inline  read() { full(); }
		void inline write() { full(); }
	}}}
	#define universalis__processor__memory_barriers__defined
#elif defined DIVERSALIS__COMPILER__GNU
	#if DIVERSALIS__COMPILER__VERSION >= 40100 // 4.1.0
		// GCC >= 4.1 has built-in intrinsics. We'll use those.
		namespace universalis { namespace processor { namespace memory_barriers {
			void inline  full() { __sync_synchronize(); }
			void inline  read() { full(); }
			void inline write() { full(); }
		}}}
		#define universalis__processor__memory_barriers__defined
	#else
		// As a fallback, GCC understands volatile asm and "memory"
		// to mean it should not reorder memory read/writes.
		#if defined DIVERSALIS__PROCESSOR__POWER_PC
			namespace universalis { namespace processor { namespace memory_barriers {
				void inline  full() { asm volatile("sync":::"memory"); }
				void inline  read() { full(); }
				void inline write() { full(); }
			}}}
			#define universalis__processor__memory_barriers__defined
		#elif defined DIVERSALIS__PROCESSOR__X86
			namespace universalis { namespace processor { namespace memory_barriers {
				void inline  full() { asm volatile("mfence":::"memory"); }
				void inline  read() { asm volatile("lfence":::"memory"); }
				void inline write() { asm volatile("sfence":::"memory"); }
			}}}
			#define universalis__processor__memory_barriers__defined
		#endif
	#endif
#elif defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION >= 1400
	#include <intrin.h>
	#pragma intrinsic(_ReadWriteBarrier)
	#pragma intrinsic(_ReadBarrier)
	#pragma intrinsic(_WriteBarrier)
	namespace universalis { namespace processor { namespace memory_barriers {
		void inline  full() { _ReadWriteBarrier(); }
		void inline  read() { _ReadBarrier(); }
		void inline write() { _WriteBarrier(); }
	}}}
	#define universalis__processor__memory_barriers__defined
#elif defined DIVERSALIS__COMPILER__MICROSOFT || defined DIVERSALIS__COMPILER__BORLAND
	namespace universalis { namespace processor { namespace memory_barriers {
		void inline  full() { _asm { lock add [esp], 0}; }
		void inline  read() { full(); }
		void inline write() { full(); }
	}}}
	#define universalis__processor__memory_barriers__defined
#endif

#if !defined universalis__processor__memory_barriers__defined
	#warning Memory barriers are not defined for this system. Will use compare_and_swap to emulate.
	#include <universalis/processor/atomic/compare_and_swap.hpp>
	namespace universalis { namespace processor { namespace memory_barriers {
		void inline  full() { static int dummy_; compare_and_swap(&dummy_, 0, 1); }
		void inline  read() {}
		void inline write() {}
	}}}
#endif
#undef universalis__processor__memory_barriers__defined
