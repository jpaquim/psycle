// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2008 members of the psycle project http://psycle.sourceforge.net ; johan boule <bohan@jabber.org>

///\implementation psycle::engine::sequence
#include <psycle/detail/project.private.hpp>
#include "sequence.hpp"
#include <cstdint>
namespace psycle { namespace engine {

//////////////////////////////////////////////////////////////////////////////////////////////////
// sequence

void sequence::erase_events(real begin_beat, real end_beat) {
	events_type::iterator last(events_.lower_bound(end_beat));
	if(last != events_.end()) --last; // exclude end_beat from range
	events_.erase(events_.lower_bound(begin_beat), last);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// sequence_iterator

sequence_iterator::sequence_iterator(typenames::sequence const & sequence)
:
	i_(sequence.events_.end()),
	sequence_(sequence_),
	beats_per_second_(1),
	seconds_per_beat_(1),
	beat_()
{}

void sequence_iterator::beat(real beat) {
	this->beat_ = beat;
	i_ = sequence_.events_.lower_bound(beat);
}

void sequence_iterator::process(buffer & out, real events_per_second, std::size_t channels) throw(exception) {
	real const samples_per_beat = events_per_second / beats_per_second_;
	std::uint64_t const initial_sample(static_cast<std::size_t>(beat_ * samples_per_beat));
	real last_beat(beat_ + (out.events() - 1) / samples_per_beat);
	std::size_t last_index(0);
	for(; i_ != sequence_.events_.end() && i_->first < last_beat; ++i_) {
		real const b(i_->first), s(i_->second);
		std::size_t const i(static_cast<std::size_t>(b * samples_per_beat - initial_sample));
		if(i < out.events()) {
			for(std::size_t c(0); c < channels; ++c) out[c][last_index](i, s);
			++last_index;
		} else { // event lost! should never happen.
			if(loggers::warning()) {
				std::ostringstream s;
				s << "event lost: " << b << ' ' << s;
				loggers::warning()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	}
	if(last_index) for(std::size_t c(0); c < channels; ++c) out[c].flag(channel::flags::discrete);
	else for(std::size_t c(0); c < channels; ++c) out[c].flag(channel::flags::empty);
	if(last_index < out.events()) for(std::size_t c(0); c < channels; ++c) out[c][last_index].index(out.events());
	beat_ = last_beat;
}

}}
