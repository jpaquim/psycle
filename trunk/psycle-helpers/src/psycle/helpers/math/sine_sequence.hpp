// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

#pragma once
#include "clip.hpp"
#if defined BOOST_AUTO_TEST_CASE
	#include <universalis/operating_system/clocks.hpp>
	#include <sstream>
	#include <cmath>
#endif
namespace psycle { namespace helpers { namespace math {

template<bool Clipped>
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
			int const swapped_index(index_ ^ 1);
			real sin;
			//if(Clipped) sin = real(0.999999) * (sequence_[index_] * step_ - sequence_[swapped_index]);
			if(Clipped) sin = clipped<real>(-1, sequence_[index_] * step_ - sequence_[swapped_index], +1);
			else sin = sequence_[index_] * step_ - sequence_[swapped_index];
			index_ = swapped_index;
			return sequence_[swapped_index] = sin;
		}
	private:
		real step_, sequence_[2];
		int index_;
};

#if defined BOOST_AUTO_TEST_CASE
	template<bool Clipped>
	void sine_sequence_test_template() {
		typedef double real;
		sine_sequence<Clipped> sin;
		{
			using namespace universalis::operating_system::clocks;
			//typedef thread clock;
			typedef monotonic clock;
			int const iterations(1000000);
			real const step(pi / 1000);
			sin(0, step);
			std::nanoseconds const t1(clock::current());
			real f1(1);
			for(int i(0); i < iterations; ++i) f1 += sin();
			std::nanoseconds const t2(clock::current());
			real f2(1), ff2(0);
			for(int i(0); i < iterations; ++i) {
				f2 += std::sin(ff2);
				ff2 += step;
			}
			std::nanoseconds const t3(clock::current());
			{
				std::ostringstream s; s << "sine_sequence<Clipped = " << Clipped << ">: " << f1;
				BOOST_MESSAGE(s.str());
			}
			{
				std::ostringstream s; s << "std::sin: " << f2;
				BOOST_MESSAGE(s.str());
			}
			{
				std::ostringstream s;
				s << (t2 - t1).get_count() * 1e-9 << "s < " << (t3 - t2).get_count() * 1e-9 << "s";
				BOOST_MESSAGE(s.str());
			}
			BOOST_CHECK(t2 - t1 < t3 - t2);
		} {
			real min(+2), max(-2);
			real step(pi);
			for(int i(0); i < 1000; ++i) {
				for(int j(0); j < 1000; ++j) {
					real const s(sin());
					if(s < min) min = s;
					if(s > max) max = s;
				}
				step *= 0.999;
				sin(0, step);
			}
			{
				std::ostringstream s;
				s << "sine_sequence<Clipped = " << Clipped << ">: min + 1: " << min + 1 << ", max - 1: " << max - 1;
				BOOST_MESSAGE(s.str());
			}
			if(Clipped) BOOST_CHECK(-1 <= min && max <= 1);
		}
	}
	BOOST_AUTO_TEST_CASE(sine_sequence_test) {
		sine_sequence_test_template<false>();
		sine_sequence_test_template<true>();
	}
#endif

}}}
