// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::plugins::sequence
#include <psycle/detail/project.private.hpp>
#include "sequence.hpp"
#include "psycle/plugins/sequence.hpp"
namespace psycle { namespace plugins {

PSYCLE__PLUGINS__NODE_INSTANTIATOR(sequence)

sequence::sequence(class plugin_library_reference & plugin_library_reference, name_type const & name)
:
	node(plugin_library_reference, name),
	sequence_iterator_(),
	out_port_(*this, "out")
{}

sequence::~sequence() {
	delete sequence_iterator_;
}

void sequence::do_process() throw(engine::exception) {
	engine::ports::output & out(*output_ports()[0]);
	if(!out) return;
	sequence_iterator()->process(out.buffer(), out.events_per_second(), out.channels());
}

}}
