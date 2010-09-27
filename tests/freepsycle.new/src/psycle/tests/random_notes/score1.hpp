// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::sequence
#pragma once
#include <psycle/engine.hpp>
#include <psycle/host.hpp>
#include <psycle/plugins/additioner.hpp>
#include <psycle/plugins/sine.hpp>
#include <psycle/plugins/decay.hpp>
#include <psycle/plugins/sequence.hpp>
#define PSYCLE__DECL  PSYCLE__TESTS__RANDOM_NOTES
#include <psycle/detail/decl.hpp>
namespace psycle { namespace tests { namespace random_notes {

class score1 {
	public:
		score1(host::plugin_resolver &, engine::graph &);
		void connect(engine::node & out);
		void generate();
	private:
		engine::sequence freq1_, freq2_, freq3_;
		engine::sequence seq1_, seq2_, seq3_;
		engine::sequence decay_seq1_, decay_seq2_, decay_seq3_;

		host::plugin_resolver & resolver_;
		engine::graph & graph_;

		plugins::additioner &additioner_;
		plugins::sine &sine1_, &sine2_, &sine3_;
		plugins::sequence &freq1_plug_, &freq2_plug_, &freq3_plug_;
		plugins::decay &decay1_, &decay2_, &decay3_;
		plugins::sequence &seq1_plug_, &seq2_plug_, &seq3_plug_;
		plugins::sequence &decay_seq1_plug_, &decay_seq2_plug_, &decay_seq3_plug_;
};

}}}
#include <psycle/detail/decl.hpp>
