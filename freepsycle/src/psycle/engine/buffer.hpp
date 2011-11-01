// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2011 members of the psycle project http://psycle.pastnotecut.org : johan boule <bohan@jabber.org>

#pragma once
#include "forward_declarations.hpp"
#include <psycle/helpers/math.hpp>
#include <vector>
#include <cassert>
#define PSYCLE__DECL  PSYCLE__ENGINE
#include <psycle/detail/decl.hpp>
namespace psycle { namespace engine {

typedef double real;

namespace math = helpers::math;

/// a sample with an associated, logical, index.
/// the index corresponds to a logical (i.e., not physical) position in a buffer.
class event {
	///\name sample
	///\{
		public:
			/// the immutable value of the sample.
			///\return immutable value of the sample
			real sample() const { return sample_; }
			operator real() const { return sample_; }

			/// the mutable value of the sample.
			///\return mutable value of the sample
			real & sample() { return sample_; }
			operator real&() { return sample_; }

			/// sets the value of the sample.
			///\param the value of the sample
			void sample(real const & sample) { sample_ = sample; }
		private:
			real sample_;
	///\}

	///\name index
	///\{
		public:
			/// the logical index of the sample.
			///\returns the logical index of the sample
			std::size_t index() const { return index_; }

			/// the mutable logical index of the sample.
			///\returns mutable value of the logical index of the sample
			std::size_t & index() { return index_; }

			/// sets the logical index of the sample.
			///\param the logical index of the sample
			void index(std::size_t index) { index_ = index; }
		private:
			std::size_t index_;
	///\}

	public:
		/// sets both the logical index and the sample value.
		void operator()(std::size_t index, real const & sample) { index_ = index; sample_ = sample; }
	
		/// compares the logical indexes of two events.
		/// The value of the sample is not taken into account.
		///\return true if this event has a logical index equal to the one of the other event.
		bool operator==(event const & event) const { return index_ == event.index_; }

		/// compares the logical indexes of two events.
		/// The value of the sample is not taken into account.
		///\return true if this event has a logical index lower than the one of the other event.
		bool operator<(event const & event) const { return index_ < event.index_; }
};

/// a vector of events.
class PSYCLE__DECL channel {
	///\name flags to give hint to process loops
	///\{
		public:
			enum class flags {
				empty, ///< indicates there is no event
				discrete, ///< indicates there are events, but not one for every sample
				continuous ///< indicates there is an event for every sample
			};
			
			flags flag() const { return flag_; }
			void flag(flags flag) { flag_ = flag; }
		private:
			flags flag_;
	///\}

	public:
		typedef class event event;

		/// creates a new zero-sized channel.
		channel() : flag_() {}

		/// creates a new channel with the given number of events.
		///\param events the number of events
		channel(std::size_t events) : flag_(), events_(events) {}

	///\name vector of events
	///\{
		private:
			typedef std::vector<event> events_type;
			events_type events_;
		public:
			events_type::size_type size() const { return events_.size(); }
			void resize(events_type::size_type events) { events_.resize(events); }
			typedef events_type::const_iterator const_iterator;
			typedef events_type::iterator iterator;
			const_iterator begin() const { return events_.begin(); }
			iterator begin() { return events_.begin(); }
			const_iterator end() const { return events_.end(); }
			iterator end() { return events_.end(); }
			events_type::const_reference operator[](events_type::size_type event) const { assert(event < size()); return events_[event]; }
			events_type::reference operator[](events_type::size_type event) { assert(event < size()); return events_[event]; }
	///\}
};

/// a vector of channels.
class PSYCLE__DECL buffer {
	public:
		typedef class channel channel;
		typedef class event event;

		/// creates a new buffer with the given number of channels and the given number of events in each channel.
		///\param channels the number of channels
		///\param events the number of events in each channel
		buffer(std::size_t channels, std::size_t events);

		/// destructor.
		virtual ~buffer() throw();

		/// the number of channels (size of the vector of channels).
		///\return the number of channels
		std::size_t channels() const { return channels_.size(); }

		/// sets the number of channels.
		///\param the number of channels
		void channels(std::size_t channels) { resize(channels, events()); }

		/// the number of events in each channels.
		///\return the number of events in each channel
		std::size_t events() const { return events_; }

		/// sets the number of events in each channel.
		///\param the number of events in each channel
		void events(std::size_t events) { resize(channels(), events); }

		/// sets the number of channels and number of events in each channel.
		///\param channels the number of channels
		///\param events the number of events in each channel
		void resize(std::size_t channels, std::size_t events);

		/// clears all events.
		/// A complexity of o(channels) is achieved by simply setting the index of the first event.
		void inline clear(std::size_t channels);

		/// copies the first given number of channels from another buffer into this buffer.
		///\param buffer the buffer to copy from
		///\param the number of channels to copy
		void inline copy(buffer const &, std::size_t channels);

	private:
		std::size_t events_;

	///\name vector of channels
	///\{
		private:
			typedef std::vector<channel> channels_type;
			channels_type channels_;
		public:
			typedef channels_type::const_iterator const_iterator;
			typedef channels_type::iterator iterator;
			const_iterator begin() const { return channels_.begin(); }
			iterator begin() { return channels_.begin(); }
			const_iterator end() const { return channels_.end(); }
			iterator end() { return channels_.end(); }
			channels_type::const_reference operator[](channels_type::size_type channel) const { assert(channel < channels()); return channels_[channel]; }
			channels_type::reference operator[](channels_type::size_type channel) { assert(channel < channels()); return channels_[channel]; }
	///\}
};

}}
#include <psycle/detail/decl.hpp>

/**********************************************************************************************************************/
// implementation details

namespace psycle { namespace engine {

void buffer::clear(std::size_t channels) {
	for(std::size_t channel(0) ; channel < channels ; ++channel) {
		(*this)[channel].flag(channel::flags::empty);
		(*this)[channel].begin()->index(events());
	}
}

void buffer::copy(buffer const & buffer, std::size_t channels) {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << "copying " << channels << " channels from buffer " << &buffer << " to buffer " << this;
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	assert("not copying itself: " && this != &buffer); // would not cause a bug, but this catches lacks of optimizations.
	for(std::size_t channel = 0; channel < channels ; ++channel) {
		(*this)[channel].flag(buffer[channel].flag());
		///\todo we can even optimise the loop with the flag information
		for(std::size_t event = 0; event < events() && buffer[channel][event].index() < events() ; ++event)
			(*this)[channel][event] = buffer[channel][event];
	}
}

}}
