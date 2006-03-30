// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::reference_counter
#pragma once
#include "named.hpp"
#include <cassert>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__REFERENCE_COUNTER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle
{
	namespace engine
	{
		/// reference counter for shared objects.
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK reference_counter
		{
		public:
			/// creates a reference counter with an initial reference count set to 0.
			inline reference_counter() : count_(0) {}
			/// deletes the reference counter
			///\pre the reference count must be 0.
			inline virtual ~reference_counter() throw() { assert(*this == 0); }
			/// increments the reference count by 1.
			inline reference_counter & operator++() throw() { ++count_; return *this; }
			/// decrements the reference count by 1.
			inline virtual reference_counter & operator--() throw() { --count_; assert(*this >= 0); return *this; }
			/// convertible to int
			///\returns the reference count.
			inline operator int const & () const throw() { return count_; }
		private:
			int count_;
		};

		/// reference counter for plugin libraries.
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK plugin_library_reference : public reference_counter
		{
		public:
			/// creates a reference counter for the library named \param name.
			inline plugin_library_reference(const std::string & name) : named_(name) {}
			/// deletes the plugin library reference counter
			inline virtual ~plugin_library_reference() throw() {}
			/// convertible to a named object.
			inline operator named const & () const throw() { return named_; }
			/// the name of the underlying referenced library.
			inline std::string const & name() const throw() { return named_.name(); }
		private:
			named const named_;
		};

		///\internal ... not used for now
		/// a reference to a shared object.
		template<typename X> class reference
		{
		public:
			inline reference(X & x) : x_(x), reference_counter_(++*new reference_counter) {}
			inline reference(reference const & other) : x_(other.x_), reference_counter_(++other.reference_counter_) {}
			inline void operator=(reference const & other) throw() { check_if_last_reference(); x_ = other.x_; reference_counter_ = ++other.reference_counter_; }
			inline ~reference() throw() { check_if_last_reference(); }
			inline operator X const & () const throw() { return *x_; }
			inline operator X & () throw() { return *x_; }
		private:
			X * x_;
			reference_counter & reference_counter_;
			void check_if_last_reference() throw()
			{
				if(--reference_counter_ == 0)
				{
					delete reference_counter_;
					delete x_;
				}
			}
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>
