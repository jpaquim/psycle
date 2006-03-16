// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// Copyright (C) 1999-2006 Johan Boule <bohan@jabber.org>
// Copyright (C) 2004-2006 Psycledelics http://psycle.pastnotecut.org

///\implementation psycle::engine::buffer
#include PACKAGENERIC__PRE_COMPILED
#include PACKAGENERIC
#include <psycle/detail/project.private.hpp>
#include "buffer.hpp"
namespace psycle
{
	namespace engine
	{
		buffer::buffer(int const & channels, int const & events) throw(std::exception)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << "new buffer " << this;
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			resize(channels, events);
		}
	
		buffer::~buffer() throw()
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << "delete buffer " << this;
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
		}
	
		void buffer::resize(int const & channels, int const & events)
		{
			if(loggers::trace()())
			{
				std::ostringstream s;
				s << "buffer " << this << " resizing to " << channels << " channels of " << events << " events each";
				loggers::trace()(s.str(), UNIVERSALIS__COMPILER__LOCATION);
			}
			assert(channels >= 0);
			assert(events >= 0);
			//assert(channels != 0 || events == 0 && "channels == 0 implies events == 0"); // why enforcing this ?
			using std::vector;
			vector< vector<event> >::resize(channels);
			this->events_ = events;
			for(iterator i(begin()) ; i != end() ; ++i) i->resize(this->events());
			clear(size());
		}
	}
}
