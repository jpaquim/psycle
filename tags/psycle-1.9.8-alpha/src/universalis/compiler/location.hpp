// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\file
///\interface universalis::operating_system::loggers
#pragma once
#include <universalis/compiler/stringized.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#if !defined DIVERSALIS__COMPILER__GNU
	// only gcc is able to include the name of the current class implicitly
	// see UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION and UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION__NO_CLASS
	#include <universalis/compiler/typenameof.hpp>
	#include <boost/current_function.hpp>
#endif
namespace universalis
{
	namespace compiler
	{
#if 1
		class location
		:
			public std::string
		{
			public:
				explicit location(std::string const & where) : std::string(where) {}
		};

		/// location of current line of the source, within       a class.
		#define UNIVERSALIS__COMPILER__LOCATION            universalis::compiler::location(UNIVERSALIS__COMPILER__LOCATION__DETAIL(UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION          ))

		/// location of current line of the source, outside of any class.
		#define UNIVERSALIS__COMPILER__LOCATION__NO_CLASS  universalis::compiler::location(UNIVERSALIS__COMPILER__LOCATION__DETAIL(UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION__NO_CLASS))

		///\internal
		//#region DETAIL
			///\internal
			#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__MARK__SEPARATOR   " # "
			#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
				///\internal
				#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__MARK__START "# "
				///\internal
				#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__MARK__END   ""
			#else
				///\internal
				#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__MARK__START "\033[34m# "
				///\internal
				#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__MARK__END   "\033[0m"
			#endif
		//#endregion
#else
		class location
		{
			public:
				location(std::string const & module, std::string const & function, std::string const & file, unsigned int const & line)
				:
					module_(module),
					function_(function),
					file_(file),
					line_(line)
				{
				}

			public:
				std::string const inline & module() const throw() { return module_; }
			private:
				std::string const          module_;

			public:
				std::string const inline & function() const throw() { return function_; }
			private:
				std::string const          function_;

			public:
				std::string const inline & file() const throw() { return file_; }
			private:
				std::string const          file_;

			public:
				unsigned int const inline & line() const throw() { return line_; }
			private:
				unsigned int const          line_;
		};
#endif
		///\internal
		//#region DETAIL
			#if defined PACKAGENERIC
				#include PACKAGENERIC
				#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__MODULE  PACKAGENERIC__MODULE__NAME " " PACKAGENERIC__PACKAGE__VERSION " " PACKAGENERIC__MODULE__VERSION
			#else
				#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__MODULE  "(unkown)"
			#endif
#if 1
			///\internal
			#define UNIVERSALIS__COMPILER__LOCATION__DETAIL(function) \
				UNIVERSALIS__COMPILER__LOCATION__DETAIL__MARK__START \
				UNIVERSALIS__COMPILER__LOCATION__DETAIL__MODULE \
				UNIVERSALIS__COMPILER__LOCATION__DETAIL__MARK__SEPARATOR \
				function \
				UNIVERSALIS__COMPILER__LOCATION__DETAIL__MARK__SEPARATOR \
				__FILE__ \
				UNIVERSALIS__COMPILER__LOCATION__DETAIL__MARK__SEPARATOR \
				UNIVERSALIS__COMPILER__STRINGIZED(__LINE__) \
				UNIVERSALIS__COMPILER__LOCATION__DETAIL__MARK__END
#else
			///\internal
			#define UNIVERSALIS__COMPILER__LOCATION__DETAIL(function) \
				location \
				( \
					UNIVERSALIS__COMPILER__LOCATION__DETAIL__MODULE, \
					function, \
					__FILE__, \
					__LINE__ \
				)
#endif

			///\internal
			#if defined DIVERSALIS__COMPILER__GNU
				// gcc is able to include the name of the current class implicitly
				// so we use the same definition in both cases
				#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION            + std::string(__PRETTY_FUNCTION__) + //\todo use direct stringizaton ; report this to boost authors
				#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION__NO_CLASS  + std::string(__PRETTY_FUNCTION__) + //\todo use direct stringizaton ; report this to boost authors
			#else
				// include the name of the current class explicitly using rtti on the "this" pointer
				#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION + universalis::compiler::typenameof(*this) + " :: " UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION__NO_CLASS
				#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION__NO_CLASS  BOOST_CURRENT_FUNCTION
			#endif
		//#endregion
	}
}
