// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::sequence
#include <psycle/detail/project.private.hpp>
#include "score1.hpp"
namespace psycle { namespace tests { namespace random_notes {

score1::score1(host::plugin_resolver & resolver, engine::graph & graph)
:
	resolver_(resolver),
	graph_(graph),
	additioner_plug_(static_cast<plugins::additioner&>(resolver("additioner", "+")))
{
	additioner_plug_.graph(graph);
	for(std::size_t i = 0; i < size; ++i) {
		{
			std::ostringstream s; s << "sine_plug_" << i;
			plugins::sine & plug(static_cast<plugins::sine&>(resolver("sine", s.str())));
			sine_plugs_.push_back(&plug);
			plug.graph(graph);
		}
		{
			std::ostringstream s; s << "decay_plug_" << i;
			plugins::decay & plug(static_cast<plugins::decay&>(resolver("decay", s.str())));
			decay_plugs_.push_back(&plug);
			plug.graph(graph);
		}
		{
			std::ostringstream s; s << "freq_seq_plug_" << i;
			plugins::sequence & plug(static_cast<plugins::sequence&>(resolver("sequence", s.str())));
			freq_seq_plugs_.push_back(&plug);
			plug.graph(graph);
		}
		{
			std::ostringstream s; s << "pulse_seq_plug_" << i;
			plugins::sequence & plug(static_cast<plugins::sequence&>(resolver("sequence", s.str())));
			pulse_seq_plugs_.push_back(&plug);
			plug.graph(graph);
		}
		{
			std::ostringstream s; s << "decay_seq_plug_" << i;
			plugins::sequence & plug(static_cast<plugins::sequence&>(resolver("sequence", s.str())));
			decay_seq_plugs_.push_back(&plug);
			plug.graph(graph);
		}
		{
			std::ostringstream s; s << "freq_seq_" << i;
			freq_seqs_.push_back(new engine::sequence(s.str()));
		}
		{
			std::ostringstream s; s << "pulse_seq_" << i;
			pulse_seqs_.push_back(new engine::sequence(s.str()));
		}
		{
			std::ostringstream s; s << "decay_seq_" << i;
			decay_seqs_.push_back(new engine::sequence(s.str()));
		}
	}
}

score1::~score1() {
	delete &additioner_plug_;
	for(std::size_t i = 0; i < size; ++i) {
		delete sine_plugs_[i];
		delete decay_plugs_[i];
		delete freq_seq_plugs_[i];
		delete pulse_seq_plugs_[i];
		delete decay_seq_plugs_[i];
	}
}

void score1::connect(engine::node & out) {
	additioner_plug_.output_port("out")->connect(*out.input_port("in"));
	for(std::size_t i = 0; i < size; ++i) {
		sine_plugs_[i]->output_port("out")->connect(*additioner_plug_.input_port("in"));
		sine_plugs_[i]->input_port("frequency")->connect(*freq_seq_plugs_[i]->output_port("out"));
		sine_plugs_[i]->input_port("amplitude")->connect(*decay_plugs_[i]->output_port("out"));
		decay_plugs_[i]->input_port("pulse")->connect(*pulse_seq_plugs_[i]->output_port("out"));
		decay_plugs_[i]->input_port("decay")->connect(*decay_seq_plugs_[i]->output_port("out"));
		freq_seq_plugs_[i]->sequence_iterator() = new engine::sequence_iterator(*freq_seqs_[i]);
		pulse_seq_plugs_[i]->sequence_iterator() = new engine::sequence_iterator(*pulse_seqs_[i]);
		decay_seq_plugs_[i]->sequence_iterator() = new engine::sequence_iterator(*decay_seqs_[i]);
	}
}

void score1::generate() {
	engine::real const beats_per_second(1);
	unsigned int const notes(100000);
	engine::real beat(0);
	engine::real duration(0.00001 / beats_per_second);
	float slowdown(0.001);
	std::vector<float> f;
	for(std::size_t i = 0; i < size; ++i) {
		f.push_back(100 + (1000. * i / size));
	}
	float ratio(1.1);
	for(unsigned int note(0); note < notes; ++note) {
		for(std::size_t i = 0; i < size; ++i) {
			engine::real const b(beat * (1 + 0.2 * i / size));
			freq_seqs_[i]->insert_event(b, f[i] * (1 + 0.17 * i / size));
			pulse_seqs_[i]->insert_event(b, 1.0 / size);
			decay_seqs_[i]->insert_event(b, 0.0001);
			f[i] *= std::pow(ratio, 1 + 2. * i / size);
			if(f[i] > 8000) { f[i] /= 150; if(i == 0) ratio *= 1.05; }
		}
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
