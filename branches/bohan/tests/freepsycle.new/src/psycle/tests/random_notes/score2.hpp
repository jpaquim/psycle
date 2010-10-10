// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2007-2010 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\interface psycle::plugins::sequence
#ifndef PSYCLE__TESTS__RANDOM_NOTES__SCORE2__INCLUDED
#define PSYCLE__TESTS__RANDOM_NOTES__SCORE2__INCLUDED
#pragma once
#include <psycle/engine.hpp>
#include <psycle/host/plugin_resolver.hpp>
#include <psycle/plugins/additioner.hpp>
#include <psycle/plugins/sine.hpp>
#include <psycle/plugins/decay.hpp>
#include <psycle/plugins/sequence.hpp>
#define PSYCLE__DECL  PSYCLE__TESTS__RANDOM_NOTES
#include <psycle/detail/decl.hpp>
namespace psycle { namespace tests { namespace random_notes {

class score2 {
	public:
		score2(host::plugin_resolver &, engine::graph &);
		~score2();
		void connect(engine::node & out);
		void generate();
	private:
		engine::sequence freq_;

		host::plugin_resolver & resolver_;
		engine::graph & graph_;

		plugins::sine & sine_;
		plugins::sequence & freq_plug_;
};

}}}
#include <psycle/detail/decl.hpp>
#endif
