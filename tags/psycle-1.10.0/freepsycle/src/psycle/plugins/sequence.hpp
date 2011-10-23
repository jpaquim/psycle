// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#pragma once
#include "plugin.hpp"
#include <psycle/engine/sequence.hpp>
#include <map>
#define PSYCLE__DECL  PSYCLE__PLUGINS__SEQUENCE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace plugins {

/// an event scheduler
class PSYCLE__DECL sequence : public node {
	public:
		sequence(class plugin_library_reference &, name_type const &);
		~sequence();
	protected:
		void do_process() /*override*/;

	public:
		engine::sequence_iterator *& sequence_iterator() { return sequence_iterator_; }
		engine::sequence_iterator const * sequence_iterator() const { return sequence_iterator_; }
	private:
		engine::sequence_iterator * sequence_iterator_;
		engine::ports::output out_port_;
};

}}
#include <psycle/detail/decl.hpp>
