// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::plugins::sequence
#include <psycle/detail/project.private.hpp>
#include "score2.hpp"
namespace psycle { namespace tests { namespace random_notes {

using engine::real;

score2::score2(host::plugin_resolver & resolver, engine::graph & graph)
:
	freq_("freq"),
	resolver_(resolver),
	graph_(graph),
	sine_(static_cast<plugins::sine&>(resolver("sine", "sine"))),
	freq_plug_(static_cast<plugins::sequence&>(resolver("sequence", freq_.name())))
{
	sine_.graph(graph);
	freq_plug_.graph(graph);
}

score2::~score2() {
	delete &sine_;
	delete &freq_plug_;
}

void score2::connect(engine::node & out) {
	freq_plug_.sequence_iterator() = new engine::sequence_iterator(freq_);
	
	sine_.output_port("out")->connect(*out.input_port("in"));
	sine_.input_port("frequency")->connect(*freq_plug_.output_port("out"));
}

void score2::generate() {
	sine_.amp(1);
	//sine_.freq(440);

	real const beats_per_second = 1;
	unsigned int const notes = 100000;
	real beat = 0;
	real duration = 0.00001 / beats_per_second;
	float slowdown = 0.001;
	float f1 = 100;
	float ratio = 1.1;
	for(unsigned int note = 0; note < notes; ++note) {
		//std::clog << beat << ' ' << f1 << '\n';
		real const b1 = beat;
		freq_.insert_event(b1, f1);
		f1 *= ratio;
		if(f1 > 8000) { f1 /= 150; ratio *= 1.05; }
		if(ratio > 1.5) ratio -= 0.5;
		if(ratio < 1.01) ratio += 0.01;
		beat += duration;
		duration += duration * slowdown;
		if(
			duration > 0.1  / beats_per_second ||
			duration < 0.00001 / beats_per_second
		) slowdown = -slowdown;
	}
}

}}}
