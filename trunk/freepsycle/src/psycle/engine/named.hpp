// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\interface psycle::engine::named
#pragma once
#include <psycle/detail/project.hpp>
#include <string>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__NAMED
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace engine {

/// anything which has a name.
class named {
	public:
		typedef std::string name_type;
		/// creates a named object.
		///\post the given name is copied.
		inline named(name_type const & name) : name_(name) {}
		///\return a const reference to the name.
		name_type const inline & name() const { return name_; }
		///\return a mutable reference to the name.
		name_type inline & name() { return name_; }
		/// sets a new name.
		///\post the given name is copied.
		void inline name(name_type const & name) { do_name(name); }
		/// overridable virtual setter.
		void virtual inline do_name(name_type const & name) { this->name_ = name; }
		virtual inline ~named() {}
	private:
		name_type name_;
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>
