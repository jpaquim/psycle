///\file
///\brief implementation file for ...
#include "Track.hpp"
namespace psycle
{
	Track::Track(const std::string& title) throw(std::exception)
	:
		title_(title)
	{
	}
	
	Track::~Track() throw()
	{
		for(iterator i(events_.begin()) ; i != events_.end() ; ++i) delete *i;
	}
	
	void Track::add_event(Event & event)
	{
		events_.insert(&event);
	}
	
	const int Track::event_count() const
	{
		return events_.size();
	}
}
