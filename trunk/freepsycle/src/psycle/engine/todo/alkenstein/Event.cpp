///\file
///\brief implementation file for ...
#include "Event.hpp"
namespace psycle
{
	Event::Event(const int & time) throw(std::exception)
	:
		timestamp_(time)
	{
	}

	Event::~Event() throw()
	{
	}
}
