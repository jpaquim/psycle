// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\file
///\brief hardware exception handling
#pragma once
#include <universalis/detail/project.hpp>
#if 0 // not needed ... the 'try' keyword let us do what we want
// we want to be able to catch hardware exceptions as typed c++ ones, not only via catch(...)
	#if defined DIVERSALIS__COMPILER__FEATURE__NOT_CONCRETE
		#define try_hard try
	#elif defined DIVERSALIS__COMPILER__GNU
		#define try_hard try
	#elif defined DIVERSALIS__COMPILER__BORLAND
		#define try_hard try
	#elif defined DIVERSALIS__COMPILER__MICROSOFT
		#define try_hard try
		// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/_core_exception_handling_topics_.28.c.2b2b29.asp
		// compiler option EH<s|a>[c[-]]>, exception handling:
			// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vccore/html/_core_.2f.Zs.asp
			// This option specifies the model of exception handling to be used by the compiler.
			// * Use /EHs to specify the synchronous exception handling model
			//   (C++ exception handling without structured exception handling exceptions).
			//   If you use /EHs, do not rely on the compiler to catch asynchronous exceptions.
			// * Use /EHa to specify the asynchronous exception handling model
			//   (C++ exception handling with structured exception handling exceptions).
			// http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/vclrfExceptionSpecifications.asp
			// function exception specification                      EHsc==GX   EHs EHsc-  EHa EHac-  EHac
			// c function (no exception specification)               throw()    throw(...) throw(...) throw(...)
			// c++ function with no exception specification          throw(...) throw(...) throw(...) throw(...)
			// c++ function with throw() exception specification     throw()    throw()    throw(...) throw(...)
			// c++ function with throw(...) exception specification  throw(...) throw(...) throw(...) throw(...)
			// c++ function with throw(type) exception specification throw(...) throw(...) throw(...) throw(...)
			//
			// notes:
			//
			// both exception handling models, synchronous and asynchronous, are fully compatible and can be mixed in the same application.
			// with both models, the compiler allows to translate hardware exceptions into c++ ones.
			//
			// (in)correctness:
			//
			// when a *hardware* exception is raised,
			// with the synchronous model, some of the unwindable objects in the function where the exception occurs may not get unwound,
			// if the compiler judges their lifetime tracking mechanics to be unnecessary for the synchronous model.
			// this problem does not occur if the asynchronous model is used, but,
			//
			// efficiency:
			//
			// the synchronous model allows the compiler to eliminate the mechanics of tracking the lifetime of certain unwindable objects,
			// and to significantly reduce the code size, if the objects' lifetimes do not overlap a function call or a throw statement.
			//
			// [bohan] conclusion: the best mode is EHsc (==GX) even if we can have some memory leaks if hardware exceptions are raised
			// [bohan] note:
			// [bohan] there is a bug in msvc concerning synchronous exception handling model ... disable it locally where necessary
			// [bohan] problems appears when using synchronous exception handling model...
			// [bohan] on msvc 6, i had to use asynchronous exception handling
			// [bohan] this is still not fixed in msvc 7.1 (it's only fixed with global optimization).
			// [bohan] it hopefully generates a warning->error, otherwise, we would have a runtime bug
			// [bohan] warning C4702: unreachable code
			// [bohan] error C2220: warning treated as error - no object file generated
			// [bohan] as of 2003-12-11 i decided not to make the compile treat the warning as an error...
			// [bohan] yet, i don't know if the compiler actually puts the code in the binary or discard it badly :/
			//
			// [bohan] shit! warning C4535: calling _set_se_translator() requires /EHa; the command line options /EHc and /GX are insufficient
	#else
		#error "Unsupported compiler ; please add support for hardware exception handling for your compiler in the file where this error is triggered."
		/*
			#elif defined DIVERSALIS__COMPILER__<your_compiler_name>
				#define try_hard ...
			#endif
		*/
	#endif
#endif // 0
