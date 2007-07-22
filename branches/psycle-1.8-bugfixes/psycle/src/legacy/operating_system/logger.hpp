#pragma once
#include <iostream>
#include <boost/thread/mutex.hpp>
#include <universalis/compiler.hpp>
#include <universalis/exception.hpp>
#include <psycle/host/Global.hpp>
///\file
///\brief logger.
namespace psycle { namespace legacy {
	using universalis::exception;

	/// logger.
	class /*UNIVERSALIS__COMPILER__DEPRECATED("universalis")*/ logger
	{
	public:
		static logger & default_logger() throw() { return default_logger_; }
		static const int & default_threshold_level() throw() { return default_threshold_level_; }
	private:
		static logger default_logger_;
		static const int default_threshold_level_ =
			#if defined OPERATING_SYSTEM__LOGGERS__THRESHOLD_LEVEL
				OPERATING_SYSTEM__LOGGERS__THRESHOLD_LEVEL
			#elif defined NDEBUG
				::psycle::host::loggers::levels::info
			#else
				::psycle::host::loggers::levels::trace
			#endif
			;
	public:
		logger(const int & threshold_level, std::ostream &);
		const int & threshold_level() const throw() { return threshold_level_; }
		const bool operator()(const int & level) const throw() { return level >= this->threshold_level_; }
		inline virtual void operator()(const int & level, const std::string & string) throw();
		boost::mutex & mutex() throw() { return mutex_; }
	protected:
		std::ostream & ostream() const throw() { return ostream_; }
	private:
		int threshold_level_;
		std::ostream & ostream_;
		boost::mutex mutex_;
	};

	class /*UNIVERSALIS__COMPILER__DEPRECATED("universalis")*/ console
	{
	public:
		static void open() throw(exception);
		static void close();
		static void log(int level, const std::string & string);
	private:
		static bool got_a_console_window_;
		console();
		~console();
		static console singleton;
	};

	// <bohan> msvc 7.1 crashes if we put this function in the implementation file instead of inlined in the header.
	inline void logger::operator()(const int & level, const std::string & string) throw()
	{
		try
		{
			if((*this)(level))
			{
				boost::mutex::scoped_lock lock(mutex());
				ostream() << "logger: " << level << ": " << string;
			}
		}
		catch(...)
		{
			boost::mutex::scoped_lock lock(mutex());
			// oh dear!
			// fallback to std::cerr
			std::cerr << "logger crashed" << std::endl;
			std::cerr << "logger: " << level << ": " << string;
		}
	}
}}
