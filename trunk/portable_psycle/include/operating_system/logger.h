#pragma once
#include <iostream>
//#include <boost/thread/mutex.hpp> // would be nice to integrate boost libs into the cvs
#include <operating_system/exception.h>
#if defined OPERATING_SYSTEM__LOGGER
	#include LIBRARY__EXPORT
#else
	#include LIBRARY__IMPORT
#endif
///\file
///\brief logger.
namespace operating_system
{
	/// logger.
	class LIBRARY logger
	{
	public:
		static inline logger & default_logger() throw() { return default_logger_; }
		static inline const int & default_threshold_level() throw() { return default_threshold_level_; }
	private:
		static logger default_logger_;
		static const int default_threshold_level_ = 1;
	public:
		logger(const int & threshold_level, std::ostream &);
		inline const int & threshold_level() const throw() { return threshold_level_; }
		inline const bool operator()(const int & level) const throw() { return level >= this->threshold_level_; }
		inline virtual void operator()(const int & level, const std::string & string) throw()
		{
			//boost::mutex::scoped_lock lock(mutex()); // scope outside the try-catch statement so that it is freed in all cases if something goes wrong.
			try
			{
				if((*this)(level))
				{
					ostream()
						<< "logger: " << level << ": "
						<< string;
				}
			}
			catch(...)
			{
				// oh dear!
				// fallback to std::cerr
				std::cerr << "logger crashed" << std::endl;
			}
		}
		//inline boost::mutex & mutex() const throw() { return mutex_; }
	protected:
		inline std::ostream & ostream() const throw() { return ostream_; }
	private:
		int threshold_level_;
		std::ostream & ostream_;
		//boost::mutex mutex_;
	};

	class LIBRARY console
	{
	public:
		console() throw(exception);
		virtual ~console() throw();
	private:
		bool got_a_console_window_;
	};
}
