// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2008-2009 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

///\implementation psycle::engine::sequence
#include <psycle/detail/project.private.hpp>
#include "sequence.hpp"
#include "buffer.hpp"
#include "psycle/engine/sequence.hpp"
#include <boost/bind.hpp>
namespace psycle { namespace engine {

//////////////////////////////////////////////////////////////////////////////////////////////////
// sequence

void sequence::insert_event(real beat, real sample) {
	events_[beat] = sample;
	changed_signal_(*this, beat, beat);
}

void sequence::erase_event(real beat) {
	events_.erase(beat);
	changed_signal_(*this, beat, beat);
}

void sequence::erase_events(real begin_beat, real end_beat) {
	events_type::iterator last(events_.lower_bound(end_beat));
	if(last != events_.end()) --last; // exclude end_beat from range
	events_.erase(events_.lower_bound(begin_beat), last);
	changed_signal_(*this, begin_beat, end_beat);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// sequence_iterator

sequence_iterator::sequence_iterator(class sequence const & s)
:
	sequence_(s),
	i_(s.events_.end()),
	sequence_changed_signal_connection_(s.changed_signal_.connect(boost::bind(&sequence_iterator::on_sequence_changed, this))),
	beats_per_second_(1),
	seconds_per_beat_(1),
	beat_()
{}

void sequence_iterator::on_sequence_changed() {
	i_ = sequence_.events_.lower_bound(beat_);
}

void sequence_iterator::beat(real beat) {
	beat_ = beat;
	i_ = sequence_.events_.lower_bound(beat);
}

void sequence_iterator::process(buffer & out, real events_per_second, std::size_t channels) throw(exception) {
	real const samples_per_beat = events_per_second * seconds_per_beat_;
	if(!samples_per_beat) return;
	uint64_t const initial_sample(static_cast<std::size_t>(beat_ * samples_per_beat));
	real last_beat(beat_ + (out.events() - 1) / samples_per_beat);
	std::size_t last_event(0), last_event_index(0);
	for(; i_ != sequence_.events_.end() && i_->first < last_beat; ++i_) {
		real const b(i_->first), s(i_->second);
		std::size_t const i(static_cast<std::size_t>(b * samples_per_beat - initial_sample));
		if(i < out.events() && last_event < out.events()) {
			for(std::size_t c(0); c < channels; ++c) out[c][last_event](i, s);
			if(i != last_event_index) {
				last_event_index = i;
				++last_event;
			}
		} else { // event lost! should never happen.
			if(loggers::warning()) {
				std::ostringstream oss;
				oss << "event lost: "
					"beat: " << b <<
					", last beat: " << last_beat <<
					", index: " << i <<
					", last event: " << last_event <<
					", last event index: " << last_event_index <<
					", out events: " << out.events() <<
					", sample: " << s;
				loggers::warning()(oss.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			--i_;
			break;
		}
	}
	if(last_event) for(std::size_t c(0); c < channels; ++c) out[c].flag(channel::flags::discrete);
	else for(std::size_t c(0); c < channels; ++c) out[c].flag(channel::flags::empty);
	if(last_event < out.events()) for(std::size_t c(0); c < channels; ++c) out[c][last_event].index(out.events());
	beat_ = last_beat;
}

}}
