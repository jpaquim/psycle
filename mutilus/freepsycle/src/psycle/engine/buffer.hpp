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
namespace psycle
{
	namespace engine
	{
		/// a vector of event.
		typedef std::vector<event> channel;

		/// a vector of channel.
		class UNIVERSALIS__COMPILER__DYNAMIC_LINK buffer : public std::vector<channel>
		{
			public:
				typedef engine::channel channel;
				/// creates a new buffer with the given number of channels and the given number of events in each channel.
				///\param channels the number of channels
				///\param events the number of events in each channel
				buffer(int const & channels, const int & events) throw(std::exception);
				/// destructor.
				virtual ~buffer() throw();
				/// the number of channels (size of the vector of channels).
				///\returns the number of channels
				int inline const size() const throw() { return std::vector< std::vector<event> >::size(); }
				/// sets the number of channels.
				///\param the number of channels
				void inline resize(const int & channels) { resize(channels, events()); }
				/// sets the number of channels and number of events in each channel.
				///\param channels the number of channels
				///\param events the number of events in each channel
				void resize(int const & channels, int const & events);
				/// the number of events in each channels.
				///\returns the number of events in each channel
				int inline const & events() const throw() { return events_; }
				/// resets all events to zeroed out values.
				void inline clear(int const & channels);
				/// copies the first given number of channels from another buffer into this buffer.
				///\param buffer the buffer to copy from
				///\param the number of channels to copy
				void inline copy(buffer const &, int const & channels);
			private:
				int events_;
		};
	}
}
#include <universalis/compiler/dynamic_link/end.hpp>

namespace psycle
{
	namespace engine
	{
		void inline buffer::clear(int const & channels)
		{
			for(int channel(0) ; channel < channels ; ++channel) (*this)[channel].begin()->index(events());
		}
	
		void inline buffer::copy(buffer const & buffer, int const & channels)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << "copying " << channels << " channels from buffer " << &buffer << " to buffer " << this;
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			assert("not copying itself: " && this != &buffer); // would not cause a bug, but this catches lacks of optimizations
			for(int channel(0) ; channel < channels ; ++channel)
				for(int event(0) ; event < events() && buffer[channel][event].index() < events() ; ++event)
					(*this)[channel][event] = buffer[channel][event];
		}
	}
}
