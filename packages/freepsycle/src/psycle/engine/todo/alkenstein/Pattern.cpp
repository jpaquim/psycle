///\file
///\brief implementation file for ...
#include <iostream>
#include <sstream>
#include "Pattern.hpp"
namespace psycle
{
	Pattern::Pattern(const std::string & title) throw(std::exception)
	:
		title_(title)
	{
		for(int x(0) ; x < initial_no_of_tracks ; ++x)
		{
			std::stringstream ss;
			ss << "track " << x;
			add_track(ss.str());
		}
	}
	
	Pattern::~Pattern() throw()
	{
		for(iterator i(tracks_.begin()) ; i != tracks_.end(); ++i) delete *i;
	}
	
	void Pattern::add_track(const std::string & title)
	{
		tracks_.push_back(new Track(title));
	}
	
	void Pattern::add_event(const int & track, Event & event)
	{
		std::cout << "Adding event to \"" << tracks_[track]->title() << "\"." << std::endl;
		tracks_[track]->add_event(event);
	}
	
	const int Pattern::event_count()
	{
		int event_count(0);
		for(iterator i(tracks_.begin()) ; i != tracks_.end() ; ++i)
			event_count += (**i).event_count();
		return event_count;
	}
}
