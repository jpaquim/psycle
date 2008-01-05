// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 1999-2007 johan boule <bohan@jabber.org>
// copyright 2004-2007 psycledelics http://psycle.pastnotecut.org

///\interface psycle::engine::buffer
#pragma once
#include "forward_declarations.hpp"
#include "event.hpp"
#include <vector>
#define UNIVERSALIS__COMPILER__DYNAMIC_LINK  PSYCLE__ENGINE__BUFFER
#include <universalis/compiler/dynamic_link/begin.hpp>
namespace psycle { namespace engine {

#if 1
typedef std::vector<event> channel;
#else
///\todo unsure whether we need a specific class for channels
/// a vector of events.
class UNIVERSALIS__COMPILER__DYNAMIC_LINK channel : public std::vector<event> {
	public:
		typedef engine::event event;

		/// creates a new channel with the given number of events.
		///\param events the number of events
		channel(std::size_t events) throw(std::exception) : std::vector<event>(events), last_() {}

		///\name index of the last event
		///\{
			/// sets the index of the last event.
			///\param last the index of the last event.
			void last(std::size_t last) throw() { last_ = last };
			/// the index of the last event.
			///\return the index of the last event.
			std::size_t last() const throw() { return last_; }
		///\}
	private:
		std::size_t last_;
};
#endif

/// a vector of channels.
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
		/// the number of channels (size of the vector of channels).
		///\return the number of channels
		std::size_t size() const throw() { return std::vector<channel>::size(); }

		/// sets the number of channels.
		///\param the number of channels
		void channels(std::size_t channels) { resize(channels, events()); }
		/// sets the number of channels.
		///\param the number of channels
		void resize(std::size_t channels) { resize(channels, events()); }

		/// sets the number of channels and number of events in each channel.
		///\param channels the number of channels
		///\param events the number of events in each channel
		void resize(std::size_t channels, std::size_t events);

		/// the number of events in each channels.
		///\return the number of events in each channel
		std::size_t events() const throw() { return events_; }

		/// clears all events.
		/// A complexity in o(channels) is achieved by simply setting the index of the last event. ///\todo
		void inline clear(std::size_t channels);

		/// copies the first given number of channels from another buffer into this buffer.
		///\param buffer the buffer to copy from
		///\param the number of channels to copy
		void inline copy(buffer const &, std::size_t channels);

	private:
		std::size_t events_;
};

}}
#include <universalis/compiler/dynamic_link/end.hpp>

/**********************************************************************************************************************/
// implementation details

namespace psycle { namespace engine {

void buffer::clear(std::size_t channels) {
	///\todo mark last or first?
	for(std::size_t channel(0) ; channel < channels ; ++channel) (*this)[channel].begin()->index(events());
}

void buffer::copy(buffer const & buffer, std::size_t channels) {
	if(loggers::trace()()) {
		std::ostringstream s;
		s << "copying " << channels << " channels from buffer " << &buffer << " to buffer " << this;
		loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
	}
	assert("not copying itself: " && this != &buffer); // would not cause a bug, but this catches lacks of optimizations.
	///\todo until last
	for(std::size_t channel(0) ; channel < channels ; ++channel)
		for(std::size_t event(0) ; event < events() && buffer[channel][event].index() < events() ; ++event)
			(*this)[channel][event] = buffer[channel][event];
}

}}

