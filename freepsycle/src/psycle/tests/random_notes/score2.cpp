// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::plugins::sequence
#include <psycle/detail/project.private.hpp>
#include "score2.hpp"
namespace psycle { namespace tests { namespace random_notes {

score2::score2(host::plugin_resolver & resolver, engine::graph & graph)
:
	resolver_(resolver),
	graph_(graph),
	sine_(static_cast<plugins::sine&>(resolver("sine", graph, "sine1").node()))
{}

void score2::connect(engine::node & out) {
	sine_.output_port("out")->connect(*out.input_port("in"));
}

void score2::generate() {
	sine_.amplitude(1);
	sine_.frequency(440);
}

}}}
