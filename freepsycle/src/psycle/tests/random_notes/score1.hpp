// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::sequence
#ifndef PSYCLE__TESTS__RANDOM_NOTES__SCORE1__INCLUDED
#define PSYCLE__TESTS__RANDOM_NOTES__SCORE1__INCLUDED
#pragma once
#include <psycle/engine.hpp>
#include <psycle/host/plugin_resolver.hpp>
#include <psycle/plugins/additioner.hpp>
#include <psycle/plugins/sine.hpp>
#include <psycle/plugins/decay.hpp>
#include <psycle/plugins/sequence.hpp>
#include <vector>
#define PSYCLE__DECL  PSYCLE__TESTS__RANDOM_NOTES
#include <psycle/detail/decl.hpp>
namespace psycle { namespace tests { namespace random_notes {

class score1 {
	public:
		score1(host::plugin_resolver &, engine::graph &);
		~score1();
		void connect(engine::node & out);
		void generate();
	private:
		std::size_t const static size = 50;
		
		std::vector<engine::sequence*> freq_seqs_;
		std::vector<engine::sequence*> pulse_seqs_;
		std::vector<engine::sequence*> decay_seqs_;

		host::plugin_resolver & resolver_;
		engine::graph & graph_;

		plugins::additioner &additioner_plug_;
		std::vector<plugins::sine*> sine_plugs_;
		std::vector<plugins::sequence*> freq_seq_plugs_;
		std::vector<plugins::decay*> decay_plugs_;
		std::vector<plugins::sequence*> pulse_seq_plugs_;
		std::vector<plugins::sequence*> decay_seq_plugs_;
};

}}}
#include <psycle/detail/decl.hpp>
#endif
