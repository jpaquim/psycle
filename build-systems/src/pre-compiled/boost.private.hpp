// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2002-2009 members of the psycle project http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\file \brief inclusions of boost headers to be pre-compiled.

#ifndef PSYCLE__BUILD_SYSTEMS__PRE_COMPILED__BOOST__INCLUDED
#define PSYCLE__BUILD_SYSTEMS__PRE_COMPILED__BOOST__INCLUDED
#pragma once

#include <diversalis/compiler.hpp>
#if defined DIVERSALIS__COMPILER__FEATURE__PRE_COMPILATION // if the compiler supports pre-compilation

	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma message("pre-compiling " __FILE__ " ...")
	#endif

	#include <boost/version.hpp>
	#include <boost/static_assert.hpp>
	//#include "boost/multi_array.hpp"
	#include <boost/filesystem/path.hpp>
	#include <boost/filesystem/operations.hpp>

	#define BOOST_THREAD_USE_DLL
	#include <boost/thread/thread.hpp>
	#include <boost/thread/condition.hpp>
	#include <boost/thread/mutex.hpp>
	#include <boost/thread/recursive_mutex.hpp>
	#if BOOST_VERSION == 103200
		#include <boost/thread/read_write_mutex.hpp>
	#endif

	// huge include! #include <boost/spirit.hpp>

	#if 0 && BOOST_VERSION >= 103301
		//#include <boost/archive/text_iarchive.hpp>
		//#include <boost/archive/text_oarchive.hpp>
		#include <boost/archive/xml_iarchive.hpp>
		#include <boost/archive/xml_oarchive.hpp>
		#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION >= 1400
			#pragma warning(push)
			#pragma warning(disable:4267) // 'argument' : conversion from 'size_t' to 'std::streamsize', possible loss of data
		#endif
		//#include <boost/archive/binary_iarchive.hpp>
		//#include <boost/archive/binary_oarchive.hpp>
		#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION >= 1400
			#pragma warning(pop)
		#endif
		//#include <boost/serialization/level.hpp>
		//#include <boost/serialization/version.hpp>
		//#include <boost/serialization/tracking.hpp>
		//#include <boost/serialization/export.hpp>
		#include <boost/serialization/nvp.hpp>
		//#include <boost/serialization/list.hpp>
		#include <boost/serialization/string.hpp>
	#endif

	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma message("pre-compiling " __FILE__ " ... done")
	#endif
#endif

#endif
