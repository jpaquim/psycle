// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::reference_counter
#pragma once
#include "named.hpp"
#include <cassert>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__REFERENCE_COUNTER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace engine {

	/// reference counter for shared objects.
	class reference_counter
	{
		public:
			/// creates a reference counter with an initial reference count set to 0.
			reference_counter() : count_() {}
			/// deletes the reference counter
			///\pre the reference count must be 0.
			virtual inline ~reference_counter() throw() { assert(!count_); }
			/// increments the reference count by 1.
			reference_counter & operator++() throw() { ++count_; return *this; }
			/// decrements the reference count by 1.
			virtual inline unsigned int operator--() throw() { assert(count_); return --count_; }
			/// convertible to unsigned int
			///\returns the reference count.
			operator unsigned int () const throw() { return count_; }
		private:
			unsigned int count_;
	};

	/// reference counter for plugin libraries.
	class plugin_library_reference : public reference_counter
	{
		public:
			/// creates a reference counter for the library named \param name.
			plugin_library_reference(std::string const & name) : named_(name) {}
			/// deletes the plugin library reference counter
			virtual inline  ~plugin_library_reference() throw() {}
			/// convertible to a named object.
			operator named const & () const throw() { return named_; }
			/// the name of the underlying referenced library.
			std::string const & name() const throw() { return named_.name(); }
		private:
			named const named_;
	};

	///\internal ... not used for now
	/// a reference to a shared object.
	template<typename X>
	class reference
	{
		public:
			reference(X & x) : x_(x), reference_counter_(++*new reference_counter) {}
			reference(reference const & other) : x_(other.x_), reference_counter_(++other.reference_counter_) {}
			void operator=(reference const & other) throw() { check_if_last_reference(); x_ = other.x_; reference_counter_ = ++other.reference_counter_; }
			~reference() throw() { check_if_last_reference(); }
			operator X const & () const throw() { return *x_; }
			operator X & () throw() { return *x_; }
		private:
			X * x_;
			reference_counter & reference_counter_;
			void check_if_last_reference() throw()
			{
				if(--reference_counter_ == 0) {
					delete reference_counter_;
					delete x_;
				}
			}
	};
}}
#include <universalis/compiler/dynamic_link/end.hpp>
