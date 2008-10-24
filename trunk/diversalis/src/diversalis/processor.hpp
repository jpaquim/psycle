// -*- mode:c++; indent-tabs-mode:t -*-
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
		/// 8 == amd64, intel emt64 "nocona".\n
		/// 7 == sse2, from late duron, atlhon-xp, intel pentium-iv.\n
		/// 6 == sse1, from pentium-pro to pentium-iii, early duron.\n
		/// 5 == i80586 aka "pentium"-i.\n
		/// 4 == i80486+487.\n
		/// 3 == i80386dx+387 (excluding the bridled i386sx).\n
		/// 2 == i80286.\n
		/// 1 == i8086 (excluding the bridled i8088).\n
		/// 0 == i8080/i8085.\n
		#define DIVERSALIS__PROCESSOR__X86 <number>
		#undef DIVERSALIS__PROCESSOR__X86 // was just defined to insert documentation.
	///\}
	
	///\name extra instruction sets for x86 processors
	///\{
		/// indicates which version of the sse instruction set the compiler's target x86 processor supports (undefined if no sse).
		#define DIVERSALIS__PROCESSOR__X86__SSE <number>
		#undef DIVERSALIS__PROCESSOR__X86__SSE // was just defined to insert documentation.
		
		/// indicates the compiler's target x86 processor supports the 3d-now instruction set.
		#define DIVERSALIS__PROCESSOR__X86__3DNOW
		#undef DIVERSALIS__PROCESSOR__X86__3DNOW // was just defined to insert documentation.
		
		/// indicates the compiler's target x86 processor supports the mmx instruction set.
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
	// gcc -E -dM -x c++ -std=c++98 -march=k8 -ffast-math -msse2 /dev/null

	#if defined __powerpc__
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__POWER_PC
	#elif defined __x86_64__ // as tested in #include <bits/wordsize.h> to #define __WORDSIZE 64
		///\todo test for __LP64__ instead
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 9
	#elif defined __k8__ || defined __nocona__
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 8
	#elif defined __pentium4__
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 7
	#elif defined __i686__
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 6
	#elif defined __i586__
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
	#elif defined __SSE2__
		#define DIVERSALIS__PROCESSOR__X86__SSE 2
	#elif defined __SSE__
		#define DIVERSALIS__PROCESSOR__X86__SSE 1
	#endif

	#if defined __3dNOW__
		#define DIVERSALIS__PROCESSOR__X86__3DNOW
	#endif

	#if defined __MMX__
		#define DIVERSALIS__PROCESSOR__X86__MMX
	#endif
			
#elif defined DIVERSALIS__COMPILER__MICROSOFT
	#if defined _M_IA64
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__IA 2
	#elif defined _M_IX86
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__X86 _M_IX86 / 100
		#if DIVERSALIS__PROCESSOR__X86 >= 7
			#define DIVERSALIS__PROCESSOR__X86__SSE 2
		#elif DIVERSALIS__PROCESSOR__X86 >= 6
			#define DIVERSALIS__PROCESSOR__X86__SSE 1
		#endif
	#elif defined _M_ALPHA
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__ALPHA_AXP _M_ALPHA
	#elif defined _M_MRX000
		#define DIVERSALIS__PROCESSOR
		#define DIVERSALIS__PROCESSOR__MRX _M_MRX000
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
