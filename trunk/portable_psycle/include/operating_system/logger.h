#pragma once
#include <iostream>
//#include <boost/thread/mutex.hpp>
#include <operating_system/exception.h>
#if defined OPERATING_SYSTEM__LOGGER
	#include LIBRARY__EXPORT
#else
	#include LIBRARY__IMPORT
#endif
///\file
///\brief logger

#define LOGGER__DEFAULT_THRESHOLD_LEVEL 0
#undef LOGGER
#if defined NDEBUG
	#define LOGGER_TO(level, ostream) if (false) ostream
#else
	#define LOGGER_TO(level, ostream) if(level >= logger::default_threshold_level()) ostream /*<< "logger: " << level << ": "*/
#endif
#define LOGGER(level) LOGGER_TO(level, std::cout)

/// logger
class LIBRARY logger
{
public:
	static inline logger & default_logger() throw() { return default_logger_; }
	static inline const int & default_threshold_level() throw() { return default_threshold_level_; }
private:
	static logger default_logger_;
	static const int default_threshold_level_ = LOGGER__DEFAULT_THRESHOLD_LEVEL;
public:
	logger(const int & threshold_level, std::ostream & ostream) throw();
	inline const int & threshold_level() const throw() { return threshold_level_; }
	inline const bool operator()(const int & level) const throw() { return level >= this->threshold_level_; }
	inline void operator()(const int & level, const std::string & string)
	{
		#if defined NDEBUG
			level, string; // not used
		#else
			if((*this)(level))
			{
				//boost::mutex::scoped_lock lock(mutex_);
				ostream_ << "logger: " << level << ' ' << string;
			}
		#endif
	}
private:
	int threshold_level_;
	//logger(const logger &);
	std::ostream & ostream_;
	//boost::mutex mutex_;
};

namespace operating_system
{
	class LIBRARY console
	{
	public:
		console() throw(exception);
		virtual ~console() throw();
	private:
		bool got_a_console_window_;
	};
}
