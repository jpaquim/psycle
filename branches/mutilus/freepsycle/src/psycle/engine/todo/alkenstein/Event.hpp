///\file
///\brief interface file for ...
#pragma once
#include <exception>
#include <set>
#include "TimeStamp.hpp"
namespace psycle
{
	class Event
	{
		public:
			Event(const int & time) throw(std::exception);
			virtual ~Event() throw();
			inline const bool operator<(const Event &) const;

		private:
			TimeStamp timestamp_;
		public:
			inline const TimeStamp & timestamp() const throw() { return timestamp_; }
	};

	inline const bool Event::operator<(const Event & that) const
	{
		return this->timestamp().time() < that.timestamp().time();
	}
}
