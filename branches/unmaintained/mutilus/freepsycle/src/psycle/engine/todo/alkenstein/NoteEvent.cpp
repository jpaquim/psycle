#include "NoteEvent.hpp"

namespace psycle
{
	NoteEvent::NoteEvent(const int & time) throw(std::exception)
	:
		Event(time)
	{
	}

	NoteEvent::~NoteEvent() throw()
	{

	}

}
