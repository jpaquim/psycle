// -*- mode:c++; indent-tabs-mode:t -*-
// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2008 psycledelics http://psycle.pastnotecut.org ; johan boule <bohan@jabber.org>

///\interface psycle::engine::buffer
#pragma once
#include "forward_declarations.hpp"
#include "event.hpp"
#include <vector>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__BUFFER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace engine {

/// a vector of events.
class UNIVERSALIS__COMPILER__DYNAMIC_LINK channel : public std::vector<event> {
	///\name flags to give hint to process loops
	///\{
		public:
			struct flags {
				enum type {
					continuous, ///< indicates there is an event for every sample
					discrete, ///< indicates there is not a event for every sample
					empty ///< indicates there is no event
				};
			};
			
			flags::type flag() const { return flag_; }
			void flag(flags::type flag) { this->flag_ = flag; }
		private:
			flags::type flag_;
	///\}

	public:
		typedef engine::event event;

		/// creates a new zero-sized channel.
		channel() throw(std::exception) : std::vector<event>(), flag_(flags::empty) {}

		/// creates a new channel with the given number of events.
		///\param events the number of events
		channel(std::size_t events) throw(std::exception) : std::vector<event>(events), flag_(flags::empty) {}
};

/// a vector of channels.
///\todo rename to polybuffer as suggested by JosepMa.
class UNIVERSALIS__COMPILER__DYNAMIC_LINK buffer : public std::vector<channel> {
	public:
		typedef engine::channel channel;
		typedef engine::event event;

		/// creates a new buffer with the given number of channels and the given number of events in each channel.
		///\param channels the number of channels
		///\param events the number of events in each channel
		buffer(std::size_t channels, std::size_t events) throw(std::exception);

		/// destructor.
		virtual ~buffer() throw();

		/// the number of channels (size of the vector of channels).
		///\return the number of channels
		std::size_t channels() const throw() { return std::vector<channel>::size(); }

		/// sets the number of channels.
		///\param the number of channels
		void channels(std::size_t channels) { resize(channels, events()); }

		/// sets the number of channels and number of events in each channel.
		///\param channels the number of channels
		///\param events the number of events in each channel
		void resize(std::size_t channels, std::size_t events);

		/// the number of events in each channels.
		///\return the number of events in each channel
		std::size_t events() const throw() { return events_; }

		/// sets the number of events in each channel.
		///\param the number of events in each channel
		void events(std::size_t events) { resize(channels(), events); }

		/// clears all events.
		/// A complexity of o(channels) is achieved by simply setting the index of the first event.
		void inline clear(std::size_t channels);

		/// copies the first given number of channels from another buffer into this buffer.
		///\param buffer the buffer to copy from
		///\param the number of channels to copy
		void inline copy(buffer const &, std::size_t channels);

	private:
		std::size_t events_;

		/// size is ambiguous because we have two dimensions: channels() and events().
		/// so we hide it by making it private.
		std::size_t size() const { return std::vector<channel>::size(); }

		/// resize is ambiguous because we have two dimensions: channels() and events().
		/// so we hide it by making it private.
		void resize(std::size_t channels) { resize(channels, events()); }
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>

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
	for(std::size_t channel(0) ; channel < channels ; ++channel) {
		(*this)[channel].flag(buffer[channel].flag());
		///\todo we can even optimise the loop with the flag information
		for(std::size_t event(0) ; event < events() && buffer[channel][event].index() < events() ; ++event)
			(*this)[channel][event] = buffer[channel][event];
	}
}

}}

