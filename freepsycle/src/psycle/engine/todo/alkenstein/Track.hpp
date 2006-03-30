///\file
///\brief interface file for ...
#pragma once
#include <exception>
#include <set>
#include <string>
#include <iterator>
#include "Event.hpp"
namespace psycle
{
	class Track
	{
	public:
		Track(const std::string & title) throw(std::exception);
		virtual ~Track() throw();
		void add_event(Event & event);
		const int event_count() const;
	
	private:
		std::string title_;
	public:
		inline const std::string & title() const throw() { return title_; }
	
	protected:
		std::set<Event*> events_;
		
	private:
		typedef std::set<Event*>::iterator iterator;
	};
}
