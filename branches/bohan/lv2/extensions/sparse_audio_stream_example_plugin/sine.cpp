// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008 psycledelics <http://psycle.sourceforge.net> ; Johan Boule <bohan@jabber.org>

///\file
///\brief example plugin that uses the sparse audio stream (SAS) LV2 extension

#pragma once
#include "../sparse_audio_stream.h"
#include <lv2plugin.hpp>
#include <stdlib.h> // <cstdlib>
#include <cmath>
namespace {

class sine : public LV2::Plugin<sine> {
	public:
		sine(double rate) : LV2::Plugin<sine>(4) {}

		enum port_indexes {
			out, in, freq, amp
		};
	
		void run(uint32_t samples);
		
		private:
			float t_;
};

void run(uint32_t samples) {
	for(uint32_t i(0); i < samples; ++i) {
		p(out)[i] = p(amp)[i] * std::sine(...);
	}
}

int static register_sine = sine::register_class("https://psycle.svn.sourceforge.net/svnroot/psycle/branches/bohan/lv2/extensions/sparse_audio_stream_example_plugin");

}

