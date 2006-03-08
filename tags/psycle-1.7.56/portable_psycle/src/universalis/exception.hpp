// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::exception
#pragma once
#include <universalis/detail/project.hpp>
#include "compiler/location.hpp"
#include <stdexcept>
#include <string>
namespace universalis
{
	/// generic exception thrown by functions of the namespace universalis.
	class exception : public std::runtime_error
	{
		public:
			exception(std::string const & what, compiler::location const & location) throw();
			virtual ~exception() throw() {}
			/// returns the message describing the cause of the exception.
			/// same as what() but returns a std::string.
			virtual operator std::string () const throw();
			compiler::location const inline & location() const throw() { return this->location_; };
		private:
			compiler::location location_;
	};
}

// causes clash with operator<<(std::ostringstream &, std::string)
//#include <sstream>
//std::ostringstream inline & operator<<(std::ostringstream & out, universalis::exception const & e) { out << e.what(); return out; }

// arch-tag: 7f6594a9-c6c9-475d-bb12-3257ad67f66e
