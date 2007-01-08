// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief project-wide processor-specific tweaks.
///\meta generic
#ifndef DIVERSALIS__PROCESSOR__INCLUDED
#define DIVERSALIS__PROCESSOR__INCLUDED
#pragma once
#include "compiler.hpp"
#include "operating_system.hpp"
//#namespace DIVERSALIS
	//#namespace PROCESSOR



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// documentation about what is defined by this file



		#if defined DIVERSALIS__COMPILER__DOXYGEN
			/// big endian byte order.
			#define DIVERSALIS__PROCESSOR__ENDIAN__BIG
			#undef DIVERSALIS__PROCESSOR__ENDIAN__BIG // was just defined to insert documentation.

			/// little endian byte order.
			#define DIVERSALIS__PROCESSOR__ENDIAN__LITTLE
			#undef DIVERSALIS__PROCESSOR__ENDIAN__LITTLE // was just defined to insert documentation.

			/// both endian byte order.
			#define DIVERSALIS__PROCESSOR__ENDIAN__BOTH
			#undef DIVERSALIS__PROCESSOR__ENDIAN__BOTH // was just defined to insert documentation.

			/// power-pc instruction set (ibm/motorola).\n
			/// 5 == g5 (970).\n
			/// 4 == g4 (7400/7410).\n
			/// 3 == g3 (750).\n
			/// 2 == 604/604e.\n
			/// 1 == 603/603e.\n
			/// 0 == 602.\n
			/// -1 == 601.\n
			#define DIVERSALIS__PROCESSOR__POWER_PC 5
			#undef DIVERSALIS__PROCESSOR__POWER_PC // was just defined to insert documentation.

			/// arm instruction set (intel(digital-e-c) / element-14 acorn/advanced risc machine, and others).\n
			/// 10 == 1110 x-scale (intel).\n
			/// 9 == 110 strong-arm (digital-e-c).\n
			/// 8 == 100 strong-arm (digital-e-c).\n
			/// 7 == arm7 (advanced risc machine (acorn & apple)).\n
			/// 6 == arm6 (advanced risc machine (acorn & apple)).\n
			/// 2 == arm2 (acorn risc machine).\n
			/// 1 == arm1 (acorn risc machine).\n
			/// -1 == 6502 (commodore(mos technology)).\n
			/// -2 == 6800 (motorola).\n
			#define DIVERSALIS__PROCESSOR__ARM 10
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
			/// 9000 == R9000 (pmc-sierra(qed)()sgi(mips)).\n
			/// 8000 == R8000 (sgi(mips)).\n
			/// 7100 == SR7100 (sand-craft()mips).\n
			/// 6000 == R6000ECL (pmc-sierra(bit()fps()intel()tektronix)).\n
			/// 5432 == R5432 (sand-craft()mips/nec).\n
			/// 5000 == R5000 (pmc-sierra(qed)()sgi(mips)).\n
			/// 4700 == R4700 (pmc-sierra(qed)()sgi(mips)).\n
			/// 4650 == R4650 (pmc-sierra(qed)()sgi(mips)).\n
			/// 4600 == R4600 (pmc-sierra(qed)()sgi(mips)).\n
			/// 4400 == R4400 (sgi(mips)).\n
			/// 4200 == R4200 (excluding the bridled R4300) (sgi(mips)).\n
			/// 4000 == R4000 (mips).\n
			/// 3010 == R3000+R3010 (mips).\n
			/// 2010 == R2000+R2010 (mips).\n
			/// 2000 == R2000 (mips).\n
			/// 1000 == R1000 (stanford).\n
			///      == (lexra).\n
			///      == Au1000 (broadcom(alchemy()dec/mips)).\n
			///      == SB1250 (amd(sibyte()dec)/mips).\n
			#define DIVERSALIS__PROCESSOR__MIPS 10
			#undef DIVERSALIS__PROCESSOR__MIPS // was just defined to insert documentation.

			/// sparc instruction set (gnu/sparc/sun(cray(fps))/texas-instruments/cypress-semiconductors/fujitsu).\n
			/// 9 == \n
			/// 8 == \n
			/// 5 == B5000ECL (pmc-sierra(bit()fps()intel()tektronix)/sun).\n
			///   == ultra-sparc-iii (sun).\n
			///   == ultra-sparc-ii (sun).\n
			///   == ultra-sparc-i (sun).\n
			///   == super-sparc (sun).\n
			/// 1 == sparc-1 (lis/weitek/sun).\n
			/// 0 == risc-ii (berkeley).\n
			/// -1 == risc-i (berkeley).\n
			#define DIVERSALIS__PROCESSOR__SPARC 9
			#undef DIVERSALIS__PROCESSOR__SPARC // was just defined to insert documentation.

			/// ia instruction set (hewlett-packard/(sgi)/intel architecture).\n
			/// 2 == itanium2.\n
			/// 1 == itanium1.\n
			/// (side-note: HP plans to retire PA and Alpha RISC CPUs to concentrate on itanium development.)\n
			#define DIVERSALIS__PROCESSOR__IA 2
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
			#define DIVERSALIS__PROCESSOR__X86 7
			#undef DIVERSALIS__PROCESSOR__X86 // was just defined to insert documentation.
		#endif



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// now the real work



		#if defined DIVERSALIS__COMPILER__GNU
			// There are so many processors supported ...
			//#if defined _X86_
				#if defined __i686__
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
			//#endif

		#elif defined DIVERSALIS__COMPILER__MICROSOFT
			#if defined _M_IA64
				#define DIVERSALIS__PROCESSOR
				#define DIVERSALIS__PROCESSOR__IA 2
			#elif defined _M_IX86
				#define DIVERSALIS__PROCESSOR
				#define DIVERSALIS__PROCESSOR__X86 _M_IX86 / 100
			#elif defined _M_ALPHA
				#define DIVERSALIS__PROCESSOR
				#define DIVERSALIS__PROCESSOR__ALPHA_AXP _M_ALPHA
			#elif defined _M_MRX000
				#define DIVERSALIS__PROCESSOR
				#define DIVERSALIS__PROCESSOR__MRX _M_MRX000
			#endif
		#endif

		//////////////
		// inferences
		//////////////

		// note: about inferences, there are less and less possible since operating systems are mostly processor-independant nowadays.

		#if !defined DIVERSALIS__PROCESSOR

			#if defined DIVERSALIS__OPERATING_SYSTEM__APPLE
				#define DIVERSALIS__PROCESSOR
				#define DIVERSALIS__PROCESSOR__POWER_PC 5
				// Implied because the version of apple's operating system for 68k processors has been discontinued.
				// Hence now, only power pc processors are supported by this operating system.
				// [bohan] Actually it seems that darwin also runs on sparc and hppa processors.

			#elif defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
				#if DIVERSALIS__COMPILER__MICROSOFT
					#if \
						DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR > 5 || \
						( \
							DIVERSALIS__OPERATING_SYSTEM__VERSION__MAJOR == 5 && \
							DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR > 0 || \
							( \
								DIVERSALIS__OPERATING_SYSTEM__VERSION__MINOR == 0 && \
								DIVERSALIS__OPERATING_SYSTEM__VERSION__PATCH > 3 \
							) \
						)
						#error "microsoft, after v5.0.3 (2ksp3), gave up with all processors but x86 and ia... no more hp(compaq(digital-e-c)) alpha axp for this operating system."
					#else
						#define DIVERSALIS__PROCESSOR
						#define DIVERSALIS__PROCESSOR__ALPHA_AXP 7 // ev7 (213?64)
					#endif
				#endif

			#endif

		#endif

		///////////////////////
		// processor endianess
		///////////////////////

		#if defined DIVERSALIS__WORDS_ENDIAN
			#if defined DIVERSALIS__WORDS_BIGENDIAN
				#define DIVERSALIS__PROCESSOR__ENDIAN
				#define DIVERSALIS__PROCESSOR__ENDIAN__BIG
			#else
				#define DIVERSALIS__PROCESSOR__ENDIAN
				#define DIVERSALIS__PROCESSOR__ENDIAN__LITTLE
			#endif
		#elif defined DIVERSALIS__PROCESSOR__POWER_PC
			#define DIVERSALIS__PROCESSOR__ENDIAN
			#define DIVERSALIS__PROCESSOR__ENDIAN__BOTH
		#elif defined DIVERSALIS__PROCESSOR__X86 || defined DIVERSALIS__PROCESSOR__IA
			#define DIVERSALIS__PROCESSOR__ENDIAN
			#define DIVERSALIS__PROCESSOR__ENDIAN__LITTLE
		#endif

		#if defined DIVERSALIS__PROCESSOR__POWER_PC && !defined DIVERSALIS__PROCESSOR__ENDIAN__BOTH
			// Since autoconf uses some real tests to determine things, it just will find the endianess that was effective during the test, but won't know it can be changed.
			#define DIVERSALIS__PROCESSOR__ENDIAN__BOTH
		#endif



		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// consistency check



		// minimum requirements for processors
		#if defined DIVERSALIS__PROCESSOR__X86 && DIVERSALIS__PROCESSOR__X86 < 3
			#error "Compiler's target processor too old."
		#endif

		#if !defined DIVERSALIS__PROCESSOR && !defined DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
			#error "Unkown processor."
		#endif

		#if !defined DIVERSALIS__PROCESSOR__ENDIAN && !defined DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
			#error "Unkown byte sex."
		#endif



	//#endnamespace
//#endnamespace
#endif
