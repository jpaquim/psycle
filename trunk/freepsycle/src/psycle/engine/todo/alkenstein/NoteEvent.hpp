#pragma once
#include <exception>
#include "TimeStamp.hpp"
#include "Event.hpp"

namespace psycle
{

	class NoteEvent : public Event
	{
		public:
			NoteEvent(const int & time) throw(std::exception);
			virtual ~NoteEvent() throw();
	};

}
