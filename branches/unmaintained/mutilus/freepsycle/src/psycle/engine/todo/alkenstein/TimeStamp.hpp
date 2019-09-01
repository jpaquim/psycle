///\file
///\brief interface file for ...
#pragma once
#include <exception>
namespace psycle
{
	class TimeStamp
	{
		public:
			TimeStamp(const int & time) throw(std::exception);
			virtual ~TimeStamp() throw();

		private:
				/// <alk> temporary, not sure of best unit for time
			int time_;
		public:
			inline const int & time() const throw() { return time_; }
	};
}
