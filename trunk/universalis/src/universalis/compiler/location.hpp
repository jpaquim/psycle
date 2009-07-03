// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\interface universalis::os::location

#ifndef UNIVERSALIS__COMPILER__LOCATION__INCLUDED
#define UNIVERSALIS__COMPILER__LOCATION__INCLUDED
#pragma once

#include <universalis/compiler/stringized.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#if !defined DIVERSALIS__COMPILER__GNU
	// Only gcc is able to include the name of the current class implicitly with __PRETTY_FUNCTION__.
	// We can use rtti support on other compilers.
	#include <universalis/compiler/typenameof.hpp>
	#include <boost/current_function.hpp>
#endif
#if defined PACKAGENERIC
	#include <packageneric.private.hpp>
#endif

namespace universalis { namespace compiler {

/// location of current line of the source, within a class.
#define UNIVERSALIS__COMPILER__LOCATION \
	universalis::compiler::location( \
		UNIVERSALIS__COMPILER__LOCATION__DETAIL(UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION) \
	)

/// location of current line of the source, outside of any class.
#define UNIVERSALIS__COMPILER__LOCATION__NO_CLASS \
	universalis::compiler::location( \
		UNIVERSALIS__COMPILER__LOCATION__DETAIL(UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION__NO_CLASS) \
	)

/// location of the source.
class location {
	public:
		location(
			std::string const & module,
			std::string const & function,
			std::string const & file,
			unsigned int const line
		)
		:
			module_(module),
			function_(function),
			file_(file),
			line_(line)
		{}

	public:  std::string const & module() const throw() { return module_; }
	private: std::string const   module_;

	public:  std::string const & function() const throw() { return function_; }
	private: std::string const   function_;

	public:  std::string const & file() const throw() { return file_; }
	private: std::string const   file_;

	public:  unsigned int       line() const throw() { return line_; }
	private: unsigned int const line_;
};
}}



/*********************************************************************************************************/
// implementation details

#if \
	defined PACKAGENERIC__MODULE__NAME && \
	defined PACKAGENERIC__MODULE__VERSION && \
	defined PACKAGENERIC__PACKAGE__VERSION
	///\internal
	#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__MODULE \
		PACKAGENERIC__MODULE__NAME " " \
		PACKAGENERIC__MODULE__VERSION  " " \
		PACKAGENERIC__PACKAGE__VERSION
#else
	///\internal
	#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__MODULE "(unkown)"
#endif

///\internal
#define UNIVERSALIS__COMPILER__LOCATION__DETAIL(function) \
	universalis::compiler::location( \
		UNIVERSALIS__COMPILER__LOCATION__DETAIL__MODULE, \
		function, \
		__FILE__, \
		__LINE__ \
	)

///\internal
#if defined DIVERSALIS__COMPILER__GNU
	// gcc is able to include the name of the current class implicitly
	// so we use the same definition in both cases
	
	#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION \
		__PRETTY_FUNCTION__

	#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION__NO_CLASS \
		__PRETTY_FUNCTION__

#else
	// include the name of the current class explicitly using rtti on the "this" pointer
	#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION \
		universalis::compiler::typenameof(*this) + " :: " UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION__NO_CLASS

	#define UNIVERSALIS__COMPILER__LOCATION__DETAIL__FUNCTION__NO_CLASS  \
		BOOST_CURRENT_FUNCTION
#endif

#endif
