// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\interface psycle::engine::named
#ifndef PSYCLE__ENGINE__NAMED__INCLUDED
#define PSYCLE__ENGINE__NAMED__INCLUDED
#pragma once
#include <psycle/detail/project.hpp>
#include <string>
#define PSYCLE__DECL  PSYCLE__ENGINE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace engine {

/// anything which has a name.
class named {
	public:
		typedef std::string name_type;
		/// creates a named object.
		///\post the given name is copied.
		named(name_type const & name) : name_(name) {}
		///\return a const reference to the name.
		name_type const & name() const { return name_; }
		///\return a mutable reference to the name.
		name_type & name() { return name_; }
		/// sets a new name.
		///\post the given name is copied.
		void name(name_type const & name) { do_name(name); }
		/// overridable virtual setter.
		virtual void do_name(name_type const & name) { this->name_ = name; }
		virtual ~named() {}
	private:
		name_type name_;
};

}}
#include <psycle/detail/decl.hpp>
#endif
