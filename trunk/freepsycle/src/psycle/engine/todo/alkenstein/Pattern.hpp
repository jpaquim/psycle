///\file
///\brief interface file for ...
#pragma once
#include <exception>
#include <vector>
#include <string>
#include "Track.hpp"
namespace psycle
{
	class Pattern
	{
	public:
		Pattern(const std::string & title) throw(std::exception);
		virtual ~Pattern() throw();
		void add_event(const int & track, Event & event);
		void add_track(const std::string & title);
		void del_track();
		const int event_count();

	private:
		std::string title_;
	public:
		inline const std::string & title() const throw() { return title_; }
		inline std::string & title() throw() { return title_; }
	
	protected:
		std::vector<Track*> tracks_;
	private:
		typedef std::vector<Track*>::iterator iterator;
		
	private:
		/// [alk] temporary, this is not the right place
		static const int initial_no_of_tracks = 16;
	};
}
