// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::plugins::sequence
#include <psycle/detail/project.private.hpp>
#include "score1.hpp"
namespace psycle { namespace tests { namespace random_notes {

score1::score1(host::plugin_resolver & resolver, engine::graph & graph)
:
	freq1_("freq1"),
	freq2_("freq2"),
	freq3_("freq3"),
	seq1_("seq1"),
	seq2_("seq2"),
	seq3_("seq3"),
	decay_seq1_("decay_seq1"),
	decay_seq2_("decay_seq2"),
	decay_seq3_("decay_seq3"),
	resolver_(resolver),
	graph_(graph),
	additioner_(static_cast<plugins::additioner&>(resolver("additioner", "+"))),
	sine1_(static_cast<plugins::sine&>(resolver("sine", "sine1"))),
	sine2_(static_cast<plugins::sine&>(resolver("sine", "sine2"))),
	sine3_(static_cast<plugins::sine&>(resolver("sine", "sine3"))),
	freq1_plug_(static_cast<plugins::sequence&>(resolver("sequence", freq1_.name()))),
	freq2_plug_(static_cast<plugins::sequence&>(resolver("sequence", freq2_.name()))),
	freq3_plug_(static_cast<plugins::sequence&>(resolver("sequence", freq3_.name()))),
	decay1_(static_cast<plugins::decay&>(resolver("decay", "decay1"))),
	decay2_(static_cast<plugins::decay&>(resolver("decay", "decay2"))),
	decay3_(static_cast<plugins::decay&>(resolver("decay", "decay3"))),
	seq1_plug_(static_cast<plugins::sequence&>(resolver("sequence", seq1_.name()))),
	seq2_plug_(static_cast<plugins::sequence&>(resolver("sequence", seq2_.name()))),
	seq3_plug_(static_cast<plugins::sequence&>(resolver("sequence", seq3_.name()))),
	decay_seq1_plug_(static_cast<plugins::sequence&>(resolver("sequence", decay_seq1_.name()))),
	decay_seq2_plug_(static_cast<plugins::sequence&>(resolver("sequence", decay_seq2_.name()))),
	decay_seq3_plug_(static_cast<plugins::sequence&>(resolver("sequence", decay_seq3_.name())))
{
	additioner_.graph(graph);
	sine1_.graph(graph);
	sine2_.graph(graph);
	sine3_.graph(graph);
	freq1_plug_.graph(graph);
	freq2_plug_.graph(graph);
	freq3_plug_.graph(graph);
	decay1_.graph(graph);
	decay2_.graph(graph);
	decay3_.graph(graph);
	seq1_plug_.graph(graph);
	seq2_plug_.graph(graph);
	seq3_plug_.graph(graph);
	decay_seq1_plug_.graph(graph);
	decay_seq2_plug_.graph(graph);
	decay_seq3_plug_.graph(graph);
}

score1::~score1() {
	delete &additioner_;
	delete &sine1_;
	delete &sine2_;
	delete &sine3_;
	delete &freq1_plug_;
	delete &freq2_plug_;
	delete &freq3_plug_;
	delete &decay1_;
	delete &decay2_;
	delete &decay3_;
	delete &seq1_plug_;
	delete &seq2_plug_;
	delete &seq3_plug_;
	delete &decay_seq1_plug_;
	delete &decay_seq2_plug_;
	delete &decay_seq3_plug_;
}

void score1::connect(engine::node & out) {
	freq1_plug_.sequence_iterator() = new engine::sequence_iterator(freq1_);
	freq2_plug_.sequence_iterator() = new engine::sequence_iterator(freq2_);
	freq3_plug_.sequence_iterator() = new engine::sequence_iterator(freq3_);
	seq1_plug_.sequence_iterator() = new engine::sequence_iterator(seq1_);
	seq2_plug_.sequence_iterator() = new engine::sequence_iterator(seq2_);
	seq3_plug_.sequence_iterator() = new engine::sequence_iterator(seq3_);
	decay_seq1_plug_.sequence_iterator() = new engine::sequence_iterator(decay_seq1_);
	decay_seq2_plug_.sequence_iterator() = new engine::sequence_iterator(decay_seq2_);
	decay_seq3_plug_.sequence_iterator() = new engine::sequence_iterator(decay_seq3_);

	additioner_.output_port("out")->connect(*out.input_port("in"));

	sine1_.output_port("out")->connect(*additioner_.input_port("in"));
	sine2_.output_port("out")->connect(*additioner_.input_port("in"));
	sine3_.output_port("out")->connect(*additioner_.input_port("in"));

	sine1_.input_port("frequency")->connect(*freq1_plug_.output_port("out"));
	sine2_.input_port("frequency")->connect(*freq2_plug_.output_port("out"));
	sine3_.input_port("frequency")->connect(*freq3_plug_.output_port("out"));

	sine1_.input_port("amplitude")->connect(*decay1_.output_port("out"));
	sine2_.input_port("amplitude")->connect(*decay2_.output_port("out"));
	sine3_.input_port("amplitude")->connect(*decay3_.output_port("out"));

	decay1_.input_port("pulse")->connect(*seq1_plug_.output_port("out"));
	decay2_.input_port("pulse")->connect(*seq2_plug_.output_port("out"));
	decay3_.input_port("pulse")->connect(*seq3_plug_.output_port("out"));

	decay1_.input_port("decay")->connect(*decay_seq1_plug_.output_port("out"));
	decay2_.input_port("decay")->connect(*decay_seq2_plug_.output_port("out"));
	decay3_.input_port("decay")->connect(*decay_seq3_plug_.output_port("out"));
}

void score1::generate() {
	engine::real const beats_per_second(1);
	unsigned int const notes(100000);
	engine::real beat(0);
	engine::real duration(0.00001 / beats_per_second);
	float slowdown(0.001);
	float f1(100), f2(400), f3(1000);
	float ratio(1.1);

	for(unsigned int note(0); note < notes; ++note) {
		//std::clog << beat << ' ' << f1 << ' ' << f2 << ' ' << f3 << '\n';

		engine::real const b1(beat), b2(beat * 1.1), b3(beat * 1.2);

		freq1_.insert_event(b1, f1);
		freq2_.insert_event(b2 * 1.1, f2 * 1.1);
		freq3_.insert_event(b3 * 1.2, f3 * 1.17);

		seq1_.insert_event(b1, 0.3);
		seq2_.insert_event(b2 * 1.1, 0.3);
		seq3_.insert_event(b3 * 1.2, 0.3);

		decay_seq1_.insert_event(b1, 0.0001);
		decay_seq2_.insert_event(b2, 0.0001);
		decay_seq3_.insert_event(b3, 0.0001);

		f1 *= ratio;
		if(f1 > 8000) { f1 /= 150; ratio *= 1.05; }
		f2 *= ratio * ratio;
		if(f2 > 8000) f2 /= 150;
		f3 *= ratio * ratio * ratio;
		if(f3 > 8000) f3 /= 150;
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
