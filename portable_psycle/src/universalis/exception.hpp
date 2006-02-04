// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2005 Psycledelics http://psycle.pastnotecut.org : Johan Boule

///\file
///\interface universalis::exception
#pragma once
#include "compiler/location.hpp"
#include <stdexcept>
#include <string>
#if !defined NDEBUG
	#include <typeinfo>
#endif
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK UNIVERSALIS__EXCEPTION
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace universalis
{
	namespace exceptions
	{
		/// holds information about the location from where an exception was triggered.
		class locatable
		{
			public:
				inline locatable(compiler::location const & location) throw() : location_(location) {}

			public:
				inline operator compiler::location const        &         () const throw() { return  location(); }
				                compiler::location const inline & location() const throw() { return *this      ; }
			private:
				                compiler::location                location_;
		};

		/// holds information about nested exceptions
		class causality
		{
			public:
				inline causality(void const * cause = 0) throw() : cause_(cause) {}

			public:
				void const inline * cause() const throw() { return this->cause_; }
			private:
				void const        * cause_;
		};
	}

	class exception
	:
		public std::exception,
		public exceptions::locatable,
		public exceptions::causality
	{
		public:
			inline exception(compiler::location const & location, void const * cause = 0) throw() : exceptions::locatable(location), exceptions::causality(cause) {}
			virtual inline ~exception() throw() {}
	};

	namespace exceptions
	{
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK runtime_error
		:
			public std::runtime_error,
			public locatable,
			public causality
		{
			public:
				runtime_error(std::string const & what, compiler::location const & location, void const * cause = 0) throw();
				virtual inline ~runtime_error() throw() {}
		};

		#if !defined NDEBUG
			class UNIVERSALIS__COMPILER__DYNAMIC_LINK bad_cast
			:
				public std::bad_cast,
				public locatable,
				public causality
			{
				public:
					bad_cast(std::type_info const & from, std::type_info const & to, compiler::location const & location, void const * cause = 0) throw();
					virtual inline ~bad_cast() throw() {}

				public:
					std::type_info const inline & from() const throw() { return this->from_; }
				private:
					std::type_info const        & from_;

				public:
					std::type_info const inline &   to() const throw() { return this->to_  ; }
				private:
					std::type_info const        &   to_;
			};
		#endif
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
// arch-tag: 7f6594a9-c6c9-475d-bb12-3257ad67f66e
