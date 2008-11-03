// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief compiler-independant meta-information about the compiler's target processor.
#ifndef DIVERSALIS__PROCESSOR__INCLUDED
#define DIVERSALIS__PROCESSOR__INCLUDED
#pragma once
#include "compiler.hpp"
#include "operating_system.hpp"

#if defined DIVERSALIS__COMPILER__DOXYGEN

	/**********************************************************************************/
	// documentation about what is defined in this file

	///\name processor endianess
	///\{
		/// indicates the compiler's targer processor uses big endian byte order.
		#define DIVERSALIS__PROCESSOR__ENDIAN__BIG
		#undef DIVERSALIS__PROCESSOR__ENDIAN__BIG // was just defined to insert documentation.

		/// indicates the compiler's targer processor uses little endian byte order.
		#define DIVERSALIS__PROCESSOR__ENDIAN__LITTLE
		#undef DIVERSALIS__PROCESSOR__ENDIAN__LITTLE // was just defined to insert documentation.

		/// indicates the compiler's targer processor is able to use both endian byte orders.
		#define DIVERSALIS__PROCESSOR__ENDIAN__BOTH
		#undef DIVERSALIS__PROCESSOR__ENDIAN__BOTH // was just defined to insert documentation.
	///\}

	/// indicates the compiler's target processor native word size in bits.
	#define DIVERSALIS__PROCESSOR__WORD_SIZE <number>
	#undef DIVERSALIS__PROCESSOR__WORD_SIZE // was just defined to insert documentation.

	///\name processor brand
	///\{
		/// power-pc instruction set (ibm/motorola).\n
		///  5 == g5 (970).\n
		///  4 == g4 (7400/7410).\n
		///  3 == g3 (750).\n
		///  2 == 604/604e.\n
		///  1 == 603/603e.\n
		///  0 == 602.\n
		/// -1 == 601.\n
		#define DIVERSALIS__PROCESSOR__POWER_PC <number>
		#undef DIVERSALIS__PROCESSOR__POWER_PC // was just defined to insert documentation.

		/// arm instruction set (intel(digital-e-c) / element-14 acorn/advanced risc machine, and others).\n
		/// 10 == 1110 x-scale (intel).\n
		///  9 == 110 strong-arm (digital-e-c).\n
		///  8 == 100 strong-arm (digital-e-c).\n
		///  7 == arm7 (advanced risc machine (acorn & apple)).\n
		///  6 == arm6 (advanced risc machine (acorn & apple)).\n
		///  2 == arm2 (acorn risc machine).\n
		///  1 == arm1 (acorn risc machine).\n
		/// -1 == 6502 (commodore(mos technology)).\n
		/// -2 == 6800 (motorola).\n
		#define DIVERSALIS__PROCESSOR__ARM <number>
		#undef DIVERSALIS__PROCESSOR__ARM // was just defined to insert documentation.

		/// mips instruction set (mips technologies incorporation).\n
		/// (MIPS I, MIPS II, MIPS III, MIPS IV, MIPS 32/64 + MIPS16 + MDMX + MIPS MT)\n
		/// 4 == mips-iv\n
		/// 3 == mips-iii\n
		/// 2 == mips-ii\n
		/// 1 == mips-i\n
		///
		/// 16000 == R16000 (mips).\n
		/// 14000 == R14000 (sgi).\n
		/// 12000 == R12000 (sgi(mips)).\n
		/// 10000 == R10000 (sgi(mips)).\n
		///  9000 == R9000 (pmc-sierra(qed)()sgi(mips)).\n
		///  8000 == R8000 (sgi(mips)).\n
		///  7100 == SR7100 (sand-craft()mips).\n
		///  6000 == R6000ECL (pmc-sierra(bit()fps()intel()tektronix)).\n
		///  5432 == R5432 (sand-craft()mips/nec).\n
		///  5000 == R5000 (pmc-sierra(qed)()sgi(mips)).\n
		///  4700 == R4700 (pmc-sierra(qed)()sgi(mips)).\n
		///  4650 == R4650 (pmc-sierra(qed)()sgi(mips)).\n
		///  4600 == R4600 (pmc-sierra(qed)()sgi(mips)).\n
		///  4400 == R4400 (sgi(mips)).\n
		///  4200 == R4200 (excluding the bridled R4300) (sgi(mips)).\n
		///  4000 == R4000 (mips).\n
		///  3010 == R3000+R3010 (mips).\n
		///  2010 == R2000+R2010 (mips).\n
		///  2000 == R2000 (mips).\n
		///  1000 == R1000 (stanford).\n
		///       == (lexra).\n
		///       == Au1000 (broadcom(alchemy()dec/mips)).\n
		///       == SB1250 (amd(sibyte()dec)/mips).\n
		#define DIVERSALIS__PROCESSOR__MIPS <number>
		#undef DIVERSALIS__PROCESSOR__MIPS // was just defined to insert documentation.

		/// sparc instruction set (gnu/sparc/sun(cray(fps))/texas-instruments/cypress-semiconductors/fujitsu).\n
		///  9 == \n
		///  8 == \n
		///  5 == B5000ECL (pmc-sierra(bit()fps()intel()tektronix)/sun).\n
		///    == ultra-sparc-iii (sun).\n
		///    == ultra-sparc-ii (sun).\n
		///    == ultra-sparc-i (sun).\n
		///    == super-sparc (sun).\n
		///  1 == sparc-1 (lis/weitek/sun).\n
		///  0 == risc-ii (berkeley).\n
		/// -1 == risc-i (berkeley).\n
		#define DIVERSALIS__PROCESSOR__SPARC <number>
		#undef DIVERSALIS__PROCESSOR__SPARC // was just defined to insert documentation.

		/// ia instruction set (hewlett-packard/(sgi)/intel architecture).\n
		/// 2 == itanium2.\n
		/// 1 == itanium1.\n
		/// (side-note: HP plans to retire PA and Alpha RISC CPUs to concentrate on itanium development.)\n
		#define DIVERSALIS__PROCESSOR__IA <number>
		#undef DIVERSALIS__PROCESSOR__IA // was just defined to insert documentation.

		/// x86 instruction set.\n
		/// 9 == 64-bit amd k8/opteron/athlon64/athlon-fx (sse2), 64-bit intel nocona/emt64.\n
		/// 8 == 32-bit intel prescott (sse3).\n
		/// 7 == 32-bit intel pentium-4 (sse2).\n
		/// 6 == 32-bit i80686 pentium-m (sse2), pentium3/athlon-4/athlon-xp/athlon-mp (sse1), althon/athlon-thunderbird (sse1 prefetch, enhanced 3d-now), k6-2/3 (3d-now), k6 (mmx), pentium2 (mmx), pentium-pro (no mmx).\n
		/// 5 == 32-bit i80586 pentium1, pentium-mmx.\n
		/// 4 == 32-bit i80486+487.\n
		/// 3 == 32-bit i80386dx+387 (excluding the bridled i386sx).\n
		/// 2 == 16-bit i80286.\n
		/// 1 == 16-bit i8086 (excluding the bridled i8088).\n
		/// 0 == 08-bit i8080/i8085.\n
		#define DIVERSALIS__PROCESSOR__X86 <number>
		#undef DIVERSALIS__PROCESSOR__X86 // was just defined to insert documentation.
	///\}
	
	///\name extra instruction sets for x86 processors
	///\{
		/// indicates which version of the sse instruction set the compiler's target x86 processor supports (undefined if no sse).
		/// 1 == sse1 (pentium3/athlon-4/athlon-xp/athlon-mp)
		/// 2 == sse2 (intel pentium-4, amd k8/opteron/athlon64/athlon-fx)
		/// 3 == sse3 (intel prescott)
		#define DIVERSALIS__PROCESSOR__X86__SSE <number>
		#undef DIVERSALIS__PROCESSOR__X86__SSE // was just defined to insert documentation.
		
		/// indicates the compiler's target x86 processor supports the 3d-now instruction set.
		/// Only amd chips have 3d-now support.
		/// 1 == 3d-now (k6-2/3)
		/// 2 == enhanced 3d-now (althon/athlon-thunderbird)
		#define DIVERSALIS__PROCESSOR__X86__3D_NOW <number>
		#undef DIVERSALIS__PROCESSOR__X86__3D_NOW // was just defined to insert documentation.
		
		/// indicates the compiler's target x86 processor supports the mmx instruction set.
		/// All i686 but pentium-pro have mmx support (the first having it are intel pentium2 and amd k6).
		/// One i586 has mmx support, the pentium-mmx.
		#define DIVERSALIS__PROCESSOR__X86__MMX
		#undef DIVERSALIS__PROCESSOR__X86__MMX // was just defined to insert documentation.
	///\}
#endif



/**********************************************************************************/
// now the real work

///////////////////
// processor brand

#if defined DIVERSALIS__COMPILER__GNU
	// There are so many processors supported ...
	// gcc -E -dM -x c++ -std=c++98 -march=<xxx> -ffast-math -msse2 /dev/null
	// where <xxx> in -march=<xxx> may be native, k8, nocona, ...

	#if defined __LP64__ ///\todo check whether that's the OS or the processor!
		#define DIVERSALIS__PROCESSOR__WORD_SIZE 64
	#endif
	
	#if defined __powerpc__
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__POWER_PC
	#elif defined __x86_64__ // amd k8/opteron/athlon64/athlon-fx (sse2), intel nocona/emt64 (sse3)
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 9
		#define DIVERSALIS__PROCESSOR__WORD_SIZE 64
	#elif \
		defined __k8__     /* amd k8/opteron/athlon64/athlon-fx (sse2) */ || \
		defined __nocona__ /* intel prescott/nocona/emt64 (sse3) */
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 8
	#elif defined __pentium4__ // <= intel pentium-4 (sse2)
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 7
	#elif \
		defined __i686__   /* intel 'pentium-m' (sse2), __pentium3__ (sse1), __pentium2__ (mmx), __pentium_pro__ (no mmx) */ || \
		defined __athlon__ /* amd althon-4/athlon-xp/athlon-mp (full sse1), athlon/athlon-thunderbird (sse1 prefetch, enhanced 3d-now) */ || \
		defined __k6__     /* amd __k6_3__/__k6_2__ (3d-now), __k6__ (mmx) */
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 6
	#elif \
		defined __i586__ /* intel __pentium_mmx__, __pentium__ */
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 5
	#elif defined __i486__
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 4
	#elif defined __i386__
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 3
	#endif

	#if defined __SSE4__
		#define DIVERSALIS__PROCESSOR__X86__SSE 4
	#elif defined __SSE3__
		#define DIVERSALIS__PROCESSOR__X86__SSE 3
		#if defined __SSE3_MATH__
			#define DIVERSALIS__PROCESSOR__X86__SSE__MATH 3
		#endif
	#elif defined __SSE2__
		#define DIVERSALIS__PROCESSOR__X86__SSE 2
		#if defined __SSE2_MATH__
			#define DIVERSALIS__PROCESSOR__X86__SSE__MATH 2
		#endif
	#elif defined __SSE__
		#define DIVERSALIS__PROCESSOR__X86__SSE 1
	#endif

	#if defined __3dNOW_A__
		#define DIVERSALIS__PROCESSOR__X86__3D_NOW 2
	#elif defined __3dNOW__
		#define DIVERSALIS__PROCESSOR__X86__3D_NOW 1
	#endif

	#if defined __MMX__
		#define DIVERSALIS__PROCESSOR__X86__MMX
	#endif
			
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	// see http://msdn.microsoft.com/en-us/library/b0084kay.aspx

	#if defined _WIN64 ///\todo that's the OS, not the processor!
		#define DIVERSALIS__PROCESSOR__WORD_SIZE 64
	#elif defined _WIN32
		#define DIVERSALIS__PROCESSOR__WORD_SIZE 32
	#endif
	
	#if defined _M_X64 // the only doc says "defined for x64 processors." ... not sure what's that suppose to mean :-(
		#define DIVERSALIS__PROCESSOR__WORD_SIZE 64
	#endif

	#if defined _M_IA64
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__IA 2
		#define DIVERSALIS__PROCESSOR__WORD_SIZE 64
	#elif defined _EMT64_ // 64-bit x86 amd or intel
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 9
		#define DIVERSALIS__PROCESSOR__X86__SSE 2 ///\todo detect SSE3
		///\todo detect 3d-now
		#define DIVERSALIS__PROCESSOR__X86__MMX
		#define DIVERSALIS__PROCESSOR__WORD_SIZE 64
	#elif defined _M_IX86 // according to the only doc, it doesn't seem to go above 600 (i686)
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 _M_IX86 / 100
		
		#if _M_IX86_FP // expands to a value indicating which -arch compiler option was used:
			// 0 if -arch was not used.
			// 1 if -arch:SSE was used.
			// 2 if -arch:SSE2 was used.
			///\todo detect SSE3 (prescott, nocona)
			#define DIVERSALIS__PROCESSOR__X86__SSE _M_IX86_FP
		#endif

		///\todo detect 3d-now

		#if DIVERSALIS__PROCESSOR__X86 >= 6
			#define DIVERSALIS__PROCESSOR__X86__MMX
		#endif
	#elif defined _M_ALPHA
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__ALPHA_AXP _M_ALPHA
	#elif defined _M_MRX000 // mips
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__MRX _M_MRX000
	#elif defined _M_PPC // xbox 360?
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__POWER_PC _M_PPC
	#endif
#endif

///////////////////////
// processor endianess

#if defined DIVERSALIS__COMPILER__GNU
	#if defined __BIG_ENDIAN__
		#define DIVERSALIS__PROCESSOR__ENDIAN
		#define DIVERSALIS__PROCESSOR__ENDIAN__BIG
	#elif defined __LITTLE_ENDIAN__
		#define DIVERSALIS__PROCESSOR__ENDIAN
		#define DIVERSALIS__PROCESSOR__ENDIAN__LITTLE
	#endif
#endif

#if !defined DIVERSALIS__PROCESSOR__ENDIAN
	#if defined DIVERSALIS__PROCESSOR__POWER_PC
		#define DIVERSALIS__PROCESSOR__ENDIAN
		#define DIVERSALIS__PROCESSOR__ENDIAN__BOTH
	#elif defined DIVERSALIS__PROCESSOR__X86 || defined DIVERSALIS__PROCESSOR__IA
		#define DIVERSALIS__PROCESSOR__ENDIAN
		#define DIVERSALIS__PROCESSOR__ENDIAN__LITTLE
	#else
		#error please specify the endianness of your processor
	#endif
#endif

#if defined DIVERSALIS__PROCESSOR__POWER_PC && !defined DIVERSALIS__PROCESSOR__ENDIAN__BOTH
	#define DIVERSALIS__PROCESSOR__ENDIAN__BOTH
#endif



/**********************************************************************************/
// consistency check



#if !defined DIVERSALIS__PROCESSOR && !defined DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
	#error "Unkown processor."
#endif

#if !defined DIVERSALIS__PROCESSOR__ENDIAN && !defined DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
	#error "Unkown byte sex."
#endif

#endif // !defined DIVERSALIS__PROCESSOR__INCLUDED
