// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include "clip.hpp"
namespace psycle { namespace helpers { namespace math {

class sine_sequence {
	public:
		/// [bohan] use 64-bit floating point numbers or else accuracy is not sufficient
		typedef double real;
		inline sine_sequence() : index_(0) {}
		inline void operator()(real phase, real radians_per_sample) throw() {
			step_ = 2 * std::cos(radians_per_sample);
			sequence_[0] = std::sin(phase - radians_per_sample);
			sequence_[1] = std::sin(phase - 2 * radians_per_sample);
			index_ = 0;
		}
		real inline operator()() throw() {
			int const swapped_index((index_ + 1) % 2);
			real const sin(clipped<real>(-1, sequence_[index_] * step_ - sequence_[swapped_index], +1));
			index_ = swapped_index;
			return sequence_[swapped_index] = sin;
		}
	private:
		real step_, sequence_[2];
		int index_;
};

}}}
